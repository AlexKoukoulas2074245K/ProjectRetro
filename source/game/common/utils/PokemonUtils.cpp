//
//  PokemonUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/07/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonUtils.h"
#include "PokemonMoveUtils.h"
#include "MathUtils.h"
#include "../GameConstants.h"
#include "../components/MoveStatsSingletonComponent.h"
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

std::unique_ptr<Pokemon> CreatePokemon
(
    const StringId pokemonName,
    const int pokemonLevel,
    const ecs::World& world
)
{
    const auto& baseStats = GetPokemonBaseStats(pokemonName, world);
    auto pokemonInstance = std::make_unique<Pokemon>(pokemonName, baseStats);

    pokemonInstance->mLevel    = pokemonLevel;    
    pokemonInstance->mXpPoints = 0;

    // Calculate IVs
    // https://bulbapedia.bulbagarden.net/wiki/Individual_values
    pokemonInstance->mAttackIv  = math::RandomInt(0, 15);
    pokemonInstance->mDefenseIv = math::RandomInt(0, 15);
    pokemonInstance->mSpeedIv   = math::RandomInt(0, 15);
    pokemonInstance->mSpecialIv = math::RandomInt(0, 15);
    pokemonInstance->mHpIv =
        ((pokemonInstance->mAttackIv & 0x1)  << 3) |
        ((pokemonInstance->mDefenseIv & 0x1) << 2) |
        ((pokemonInstance->mSpeedIv & 0x1)   << 1) |
        ((pokemonInstance->mSpecialIv & 0x1  << 0));

    // Reset EVs
    pokemonInstance->mHpEv      = 0;
    pokemonInstance->mAttackEv  = 0;
    pokemonInstance->mDefenseEv = 0;
    pokemonInstance->mSpeedEv   = 0;
    pokemonInstance->mSpecialEv = 0;

    // Calculate Hp Stat
    pokemonInstance->mMaxHp  = CalculateHpStat(pokemonInstance->mLevel, baseStats.mHp, pokemonInstance->mHpIv, pokemonInstance->mHpEv);
    pokemonInstance->mHp     = pokemonInstance->mMaxHp;

    // Calculate other stats
    pokemonInstance->mAttack  = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseStats.mAttack, pokemonInstance->mAttackIv, pokemonInstance->mAttackEv);
    pokemonInstance->mDefense = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseStats.mDefense, pokemonInstance->mDefenseIv, pokemonInstance->mDefenseEv);
    pokemonInstance->mSpeed   = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseStats.mSpeed, pokemonInstance->mSpeedIv, pokemonInstance->mSpeedEv);
    pokemonInstance->mSpecial = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseStats.mSpecial, pokemonInstance->mSpecialIv, pokemonInstance->mSpecialEv);

    // Reset encounter stage modifiers
    pokemonInstance->mAttackEncounterStage  = 0;
    pokemonInstance->mDefenseEncounterStage = 0;
    pokemonInstance->mSpeedEncounterStage   = 0;
    pokemonInstance->mSpecialEncounterStage = 0;

    // Popuplate moveset
    auto nextInsertedMoveIndex = 0U;
    for (const auto& moveLearnInfo: baseStats.mLearnset)
    {
        if (moveLearnInfo.mLevelLearned > pokemonLevel)
        {
            continue;
        }

        const auto& moveStats = GetMoveStats(moveLearnInfo.mMoveName, world);

        pokemonInstance->mMoveSet[nextInsertedMoveIndex] = std::make_unique<PokemonMoveStats>
        (
            moveStats.mName,
            moveStats.mType,
            moveStats.mPower,
            moveStats.mAccuracy,
            moveStats.mTotalPowerPoints
        );

        nextInsertedMoveIndex = (nextInsertedMoveIndex + 1) % 4;
    }

    return pokemonInstance;
}

int CalculateHpStat
(
    const int pokemonLevel,
    const int pokemonHpBaseStat,
    const int pokemonHpIv,
    const int pokemonHpEv
)
{
    // https://bulbapedia.bulbagarden.net/wiki/Statistic#cite_note-2
    const auto sqrtTerm = static_cast<int>(math::Sqrt(static_cast<float>(pokemonHpEv))/4.0f);
    const auto divTerm = static_cast<int>((((pokemonHpBaseStat + pokemonHpIv) * 2 + sqrtTerm) * pokemonLevel)/100.0f);
    return divTerm + pokemonLevel + 10;
}

int CalculateStatOtherThanHp
(
    const int pokemonLevel,
    const int pokemonBaseStat,
    const int pokemonStatIv,
    const int pokemonStatEv
)
{
    // https://bulbapedia.bulbagarden.net/wiki/Statistic#cite_note-2
    const auto sqrtTerm = static_cast<int>(math::Sqrt(static_cast<float>(pokemonStatEv)) / 4.0f);
    const auto divTerm = static_cast<int>((((pokemonBaseStat + pokemonStatIv) * 2 + sqrtTerm) * pokemonLevel) / 100.0f);
    return divTerm + 5;
}

const PokemonBaseStats& GetPokemonBaseStats
(
    const StringId pokemonName,
    const ecs::World& world
)
{
    const auto& pokemonBaseStatsComponent = world.GetSingletonComponent<PokemonBaseStatsSingletonComponent>();
    return pokemonBaseStatsComponent.mPokemonBaseStats.at(pokemonName);
}

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
        const auto pokemonName  = StringId(it.key());
        const auto& statsObject = it.value();

        const auto id              = statsObject["id"].get<int>();
        const auto hp              = statsObject["hp"].get<int>();
        const auto attack          = statsObject["attack"].get<int>();
        const auto defense         = statsObject["defense"].get<int>();
        const auto speed           = statsObject["speed"].get<int>();
        const auto special         = statsObject["special"].get<int>();
        const auto xpStat          = statsObject["xpstat"].get<int>();
        const auto catchRate       = statsObject["catchrate"].get<int>();
        const auto overworldSprite = overworldSpriteNamesToEnumValues.at(statsObject["ovimagetype"].get<std::string>());

        const auto& typeStringsArray = statsObject["type"];
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

        std::vector<MoveLearnInfo> learnset;
        for (const auto& moveLearnInfo : statsObject["moveset"])
        {
            learnset.push_back
            ({
                StringId(moveLearnInfo["movename"].get<std::string>()),
                moveLearnInfo["level"].get<int>()
            });
        }

        pokemonBaseStats.insert(std::make_pair(pokemonName, PokemonBaseStats
        (
            evolutions,
            learnset,            
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

