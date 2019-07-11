//
//  PokemonMoveUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonMoveUtils.h"
#include "MathUtils.h"
#include "../GameConstants.h"
#include "../components/MoveStatsSingletonComponent.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <json.hpp>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string MOVE_STATS_FILE_NAME         = "moves.json";
static const std::string TYPE_EFFECTIVENESS_FILE_NAME = "type_effectiveness.json";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

size_t GetFindFirstUnusedMoveIndex(const PokemonMoveSet& moveset)
{
    for (auto i = 0U; i < moveset.size(); ++i)
    {
        if (moveset[i] == nullptr)
        {
            return i;
        }
    }

    return moveset.size();
}

void LoadAndPopulateMoveStats(MoveStatsSingletonComponent& moveStatsComponent)
{
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    // Get move stats data file resource
    const auto moveStatsFilePath = ResourceLoadingService::RES_DATA_ROOT + MOVE_STATS_FILE_NAME;
    resourceLoadingService.LoadResource(moveStatsFilePath);
    const auto& moveStatsFileResource = resourceLoadingService.GetResource<DataFileResource>(moveStatsFilePath);

    // Parse base stats json
    const auto moveStatsJson = nlohmann::json::parse(moveStatsFileResource.GetContents());

    for (auto it = moveStatsJson.begin(); it != moveStatsJson.end(); ++it)
    {
        const auto moveName = StringId(it.key());
        const auto& statsObject = it.value();

        moveStatsComponent.mMoveStats.insert(std::make_pair(moveName, PokemonMoveStats
        (
            moveName,
            StringId(statsObject["type"].get<std::string>()),
            statsObject["power"].get<int>(),
            statsObject["accuracy"].get<int>(),
            statsObject["pp"].get<int>()
        )));
    }
    
    // Get type effectiveness chart data file resource
    const auto typeEffectivenessFilePath = ResourceLoadingService::RES_DATA_ROOT + TYPE_EFFECTIVENESS_FILE_NAME;
    resourceLoadingService.LoadResource(typeEffectivenessFilePath);
    const auto& typeEffectivenessFileResource = resourceLoadingService.GetResource<DataFileResource>(typeEffectivenessFilePath);
    
    // Parse type effectiveness json
    const auto typeEffectivenessJson = nlohmann::json::parse(typeEffectivenessFileResource.GetContents());
    
    for (auto it = typeEffectivenessJson.begin(); it != typeEffectivenessJson.end(); ++it)
    {
        const auto referenceType = StringId(it.key());
        for (auto innerIt = it.value().begin(); innerIt != it.value().end(); ++innerIt)
        {
            const auto targetType         = StringId(innerIt.key());
            const auto effectivenessValue = innerIt.value();
            
            moveStatsComponent.mMoveEffectivenessChart[referenceType][targetType] = effectivenessValue;
        }
    }
}

float GetTypeEffectiveness
(
    const StringId attackingMoveType,
    const StringId defendingMoveType,
    const ecs::World& world
)
{
    const auto& moveStatsComponent = world.GetSingletonComponent<MoveStatsSingletonComponent>();
    return moveStatsComponent.mMoveEffectivenessChart.at(attackingMoveType).at(defendingMoveType);
}

const PokemonMoveStats& GetMoveStats
(
    const StringId moveName,
    const ecs::World& world
)
{
    const auto& moveStatsComponent = world.GetSingletonComponent<MoveStatsSingletonComponent>();
    return moveStatsComponent.mMoveStats.at(moveName);
}

int CalculateDamage
(
    const int attackingPokemonLevel,
    const int attackingMovePower,
    const int attackingPokemonAttackingStat, // Attack or Special
    const int defendingPokemonDefensiveStat, // Defense or Special
    const float effectivenessFactor,
    const bool isStab
)
{
    const auto innerFractionTerm         = static_cast<int>((2 * attackingPokemonLevel)/5.0f);
    const auto attackDefenseFractionTerm = static_cast<int>(attackingPokemonAttackingStat/static_cast<float>(defendingPokemonDefensiveStat));
    const auto outerFractionTerm         = static_cast<int>(((innerFractionTerm + 2) * attackingMovePower * attackDefenseFractionTerm)/50.0f);
    return static_cast<int>((outerFractionTerm + 2) * (isStab ? 1.5f : 1.0f) * effectivenessFactor * math::RandomInt(217, 255)/255.0f);

}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

