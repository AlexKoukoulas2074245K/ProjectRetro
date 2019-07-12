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
#include <unordered_set>

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

bool ShouldMoveMiss
(
    const int moveAccuracy,
    const int attackerAccuracyStage,
    const int defenderEvasionStage
)
{
    // Gen 1 Miss
    if (math::RandomInt(0, 255) == 255)
    {
        return true;
    }

    const auto accuracyFactor = attackerAccuracyStage > 0 ? (attackerAccuracyStage + 3)/3.0f : (3.0f/(attackerAccuracyStage + 3));
    const auto evasionFactor  = defenderEvasionStage > 0  ? (defenderEvasionStage + 3)/3.0f  : (3.0f/(defenderEvasionStage + 3));

    const auto moveLandingProbability = (moveAccuracy/255.0f) * accuracyFactor/evasionFactor;
    return math::RandomInt(0, 100) > static_cast<int>(moveLandingProbability * 100);
}

bool ShouldMoveCrit
(
    const StringId moveName,
    const int attackerSpeed
)
{
    // https://bulbapedia.bulbagarden.net/wiki/Critical_hit
    static const std::unordered_set<StringId, StringIdHasher> highCritMoveNames = 
    {
        StringId("CRABHAMMER"), StringId("SLASH"), StringId("KARATE_CHOP"), StringId("RAZOR_LEAF")
    };

    auto thresholdValue = attackerSpeed * 0.5f;

    // Focus energy bug
    if (moveName == StringId("FOCUS_ENERGY"))
    {
        thresholdValue *= 0.25f;
    }
    else if (highCritMoveNames.count(moveName) != 0)
    {
        thresholdValue *= 8.0f;
    }

    const auto finalThresholdValue = static_cast<int>(thresholdValue);
        
    return math::RandomInt(0, 255) < finalThresholdValue;
}

int CalculateDamage
(
    const int attackingPokemonLevel,
    const int attackingMovePower,
    const int attackingPokemonAttackingStat, // Attack or Special
    const int defendingPokemonDefensiveStat, // Defense or Special
    const float effectivenessFactor,
    const bool isCrit,
    const bool isStab
)
{
    // https://bulbapedia.bulbagarden.net/wiki/Damage
    const auto innerFractionTerm = static_cast<int>((2 * (isCrit ? 2 : 1) * attackingPokemonLevel)/5.0f) + 2;
    const auto numeratorTerm     = static_cast<int>(innerFractionTerm * attackingMovePower * attackingPokemonAttackingStat);
    const auto denominatorTerm   = static_cast<int>(50 * defendingPokemonDefensiveStat);
    const auto fractionResult    = static_cast<int>(numeratorTerm / static_cast<float>(denominatorTerm)) + 2;
    const auto modifiers         = (isStab ? 1.5f : 1.0f) * effectivenessFactor * (math::RandomInt(217, 255) / 255.0f);    
    return static_cast<int>(fractionResult * modifiers);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

