//
//  PokemonStatsUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/07/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonStatsUtils.h"
#include "../components/PokemonBaseStatsSingletonComponent.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <json.hpp>
#include <unordered_map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string BASE_STATS_FILE_NAME = "base_stats.json";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void LoadAndPopulatePokemonBaseStats
(
    PokemonBaseStatsSingletonComponent& pokemonBaseStatsComponent
)
{
    static const std::unordered_map<StringId, OverworldPokemonSpriteType, StringIdHasher> overworldSpriteNamesToEnumValues =
    {
        { StringId("BALL"),    OverworldPokemonSpriteType::BALL },
        { StringId("BEAST"),   OverworldPokemonSpriteType::BEAST },
        { StringId("BUG"),     OverworldPokemonSpriteType::BUG },
        { StringId("GRASS"),   OverworldPokemonSpriteType::GRASS },
        { StringId("DRAGON"),  OverworldPokemonSpriteType::DRAGON },
        { StringId("FLYING"),  OverworldPokemonSpriteType::FLYING },
        { StringId("FOSSIL"),  OverworldPokemonSpriteType::FOSSIL },
        { StringId("NORMAL"),  OverworldPokemonSpriteType::NORMAL },
        { StringId("PIKACHU"), OverworldPokemonSpriteType::PIKACHU },
        { StringId("WATER"),   OverworldPokemonSpriteType::WATER },
    };

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    // Get level data file resource
    const auto levelFilePath = ResourceLoadingService::RES_DATA_ROOT + BASE_STATS_FILE_NAME;
    resourceLoadingService.LoadResource(levelFilePath);
    const auto& levelFileResource = resourceLoadingService.GetResource<DataFileResource>(levelFilePath);

    // Parse level json
    const auto levelJson = nlohmann::json::parse(levelFileResource.GetContents());
    
    auto& pokemonBaseStats = pokemonBaseStatsComponent.mPokemonBaseStats;
    for (auto it = levelJson.begin(); it != levelJson.end(); ++it)
    {
        auto& statsObject = it.value();

        const auto id              = statsObject["id"].get<int>();
        const auto hp              = statsObject["hp"].get<int>();
        const auto attack          = statsObject["attack"].get<int>();
        const auto defense         = statsObject["defense"].get<int>();
        const auto speed           = statsObject["speed"].get<int>();
        const auto special         = statsObject["special"].get<int>();
        const auto xpStat          = statsObject["xpstat"].get<int>();
        const auto catchRate       = statsObject["catchrate"].get<int>();
        const auto overworldSprite = overworldSpriteNamesToEnumValues.at(statsObject["ovimagetype"].get<std::string>());

        const auto typeStringsArray = statsObject["type"];
        auto firstType  = StringId();
        auto secondType = StringId();

        firstType = StringId(typeStringsArray[0]["name"].get<std::string>());

        if (typeStringsArray.size() == 2)
        {
            secondType = StringId(typeStringsArray[1]["name"].get<std::string>());
        }
        
        std::vector<EvolutionInfo> evolutions;
        for (const auto& evolutionInfo : statsObject["evolution"])
        {
            evolutions.push_back
            ({
                StringId(evolutionInfo["name"].get<std::string>()),
                StringId(evolutionInfo["method"].get<std::string>())
            });
        }

        pokemonBaseStats.insert(std::make_pair(StringId(it.key()), PokemonBaseStats
        (
            evolutions,
            StringId(it.key()),
            firstType, 
            secondType, 
            id,
            hp,
            attack, 
            defense, 
            speed, 
            special,
            xpStat, 
            catchRate, 
            overworldSprite
        )));
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

