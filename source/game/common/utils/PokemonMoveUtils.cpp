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
#include "PokemonUtils.h"
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

static const StringId FASTEST_MOVE_NAME = StringId("QUICK_ATTACK");

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

size_t FindFirstUnusedMoveIndex(const PokemonMoveSet& moveset)
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

        const auto effect = statsObject.find("effect") != statsObject.end() ? StringId(statsObject["effect"].get<std::string>()) : StringId();

        moveStatsComponent.mMoveStats.insert(std::make_pair(moveName, PokemonMoveStats
        (
            moveName,
            StringId(statsObject["type"].get<std::string>()),
            effect,
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

bool IsMoveSpecial
(
    const StringId moveType
)
{
    return moveType == StringId("WATER")
        || moveType == StringId("GRASS")
        || moveType == StringId("FIRE")
        || moveType == StringId("ICE")
        || moveType == StringId("ELECTRIC")
        || moveType == StringId("PSYCHIC")
        || moveType == StringId("DRAGON")
        ;    
}

bool IsMoveNonShake
(
    const StringId moveName
)
{
    return moveName == POISON_TICK_MOVE_NAME
        || moveName == StringId("THUNDER_WAVE")
        || moveName == StringId("HARDEN")
        ;
}


bool DoesMoveHaveSpeciallyHandledAnimation
(
    const StringId moveName
)
{
    return moveName == StringId("LEER")
        || moveName == StringId("HARDEN")
        || moveName == StringId("BIDE")
        || moveName == StringId("QUICK_ATTACK")
        || moveName == StringId("TACKLE")
        || moveName == StringId("TAIL_WHIP")
        || moveName == StringId("CONFUSION")
        ;
}

bool IsMoveAffectedByWhiteFlipEffect
(
    const StringId moveName
)
{
    return moveName == StringId("BIDE")
        ;
}

bool ShouldSfxBeSkippedForMove
(
    const StringId moveName
)
{
    return moveName == StringId("BIDE")
        ;
}

bool DoesMovesetHaveMove
(
    const StringId moveName,
    const PokemonMoveSet& moveset
)
{
    for (auto i = 0U; i < 4; ++i)
    {
        if (moveset[i] == nullptr)
        {
            break;
        }
        else if (moveset[i]->mName == moveName)
        {
            return true;
        }
    }

    return false;
}

bool ShouldAttackingPokemonBeFullyParalyzed
(
    const Pokemon& pokemon
)
{
    if (pokemon.mStatus == PokemonStatus::PARALYZED)
    {
        return math::RandomInt(0, 3) == 3;
    }
    return false;
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

    const auto accuracyFactor = attackerAccuracyStage > 0 ? (attackerAccuracyStage + 3)/3.0f : (3.0f/(-attackerAccuracyStage + 3));
    const auto evasionFactor  = defenderEvasionStage > 0  ? (defenderEvasionStage + 3)/3.0f  : (3.0f/(-defenderEvasionStage + 3));

    const auto moveLandingProbability = (moveAccuracy/255.0f) * accuracyFactor/evasionFactor;
    return math::RandomInt(0, 100) > static_cast<int>(moveLandingProbability * 100);
}

bool ShouldMoveCrit
(
    const StringId moveName,    
    const int attackerBaseSpeed
)
{   
    // https://bulbapedia.bulbagarden.net/wiki/Critical_hit
    static const std::unordered_set<StringId, StringIdHasher> highCritMoveNames = 
    {
        StringId("CRABHAMMER"), StringId("SLASH"), StringId("KARATE_CHOP"), StringId("RAZOR_LEAF")
    };
    
    auto thresholdValue = attackerBaseSpeed * 0.5f;

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

bool ShouldOpponentGoFirst
(
    const StringId playersMoveName,
    const StringId opponentsMoveName,
    const int playersPokemonSpeedModifier,
    const int opponentsPokemonSpeedModifier,
    const int playersPokemonSpeed,
    const int opponentsPokemonSpeed,
    const PokemonStatus playerPokemonStatus,
    const PokemonStatus opponentPokemonStatus
)
{
    auto modifiedPlayerSpeed   = GetStatWithModifierApplied(playersPokemonSpeed, playersPokemonSpeedModifier);
    auto modifiedOpponentSpeed = GetStatWithModifierApplied(opponentsPokemonSpeed, opponentsPokemonSpeedModifier);

    if (playerPokemonStatus == PokemonStatus::PARALYZED)
    {
        modifiedPlayerSpeed /= 4;
    }
    if (opponentPokemonStatus == PokemonStatus::PARALYZED)
    {
        modifiedOpponentSpeed /= 4;
    }
    
    if (playersMoveName == FASTEST_MOVE_NAME && opponentsMoveName != FASTEST_MOVE_NAME)
    {
        return false;
    }
    else if (playersMoveName != FASTEST_MOVE_NAME && opponentsMoveName == FASTEST_MOVE_NAME)
    {
        return true;
    }
    else if (modifiedPlayerSpeed == modifiedOpponentSpeed)
    {
        return math::RandomSign() == 1;
    }
    else
    {
        return modifiedOpponentSpeed > modifiedPlayerSpeed;
    }
}

int CalculateDamage
(
    const int attackingPokemonLevel,
    const int attackingMovePower,
    const int attackingStatModifier,
    const int defensiveStatModifier,
    const int attackingPokemonAttackingStat, // Attack or Special
    const int defendingPokemonDefensiveStat, // Defense or Special
    const float effectivenessFactor,
    const bool isCrit,
    const bool isStab
)
{
    // https://bulbapedia.bulbagarden.net/wiki/Damage
    const auto modifiedAttackStat    = isCrit ? attackingPokemonAttackingStat : GetStatWithModifierApplied(attackingPokemonAttackingStat, attackingStatModifier);
    const auto modifiedDefensiveStat = isCrit ? defendingPokemonDefensiveStat : GetStatWithModifierApplied(defendingPokemonDefensiveStat, defensiveStatModifier);

    const auto innerFractionTerm = static_cast<int>((2 * (isCrit ? 2 : 1) * attackingPokemonLevel)/5.0f) + 2;
    const auto numeratorTerm     = static_cast<int>(innerFractionTerm * attackingMovePower * modifiedAttackStat);
    const auto denominatorTerm   = static_cast<int>(50 * modifiedDefensiveStat);
    const auto fractionResult    = static_cast<int>(numeratorTerm / static_cast<float>(denominatorTerm)) + 2;
    const auto modifiers         = (isStab ? 1.5f : 1.0f) * effectivenessFactor * (math::RandomInt(217, 255) / 255.0f);    
    return math::Max(1, static_cast<int>(fractionResult * modifiers));
}

int CalculatePokemonHurtingItselfDamage
(
    const int pokemonLevel,
    const int pokemonAttackingStat,
    const int pokemonDefendingStat
)
{
    // https://bulbapedia.bulbagarden.net/wiki/Status_condition#Confusion    
    const auto innerFractionTerm = static_cast<int>((2 * pokemonLevel) / 5.0f) + 2;
    const auto numeratorTerm     = static_cast<int>(innerFractionTerm * 40 * pokemonAttackingStat);
    const auto denominatorTerm   = static_cast<int>(50 * pokemonDefendingStat);
    const auto fractionResult    = static_cast<int>(numeratorTerm / static_cast<float>(denominatorTerm)) + 2;
    const auto modifiers         = math::RandomInt(217, 255) / 255.0f;
    return static_cast<int>(fractionResult * modifiers);
}

int CalculateBindOrWrapRoundDuration
(

)
{
    //https://bulbapedia.bulbagarden.net/wiki/Bind_(move)
    //https://bulbapedia.bulbagarden.net/wiki/Wrap_(move)
    const auto rng = math::RandomInt(0, 1000);

    if (rng <= 375)
    {
        return 1;
    }    
    else if (rng <= 750)
    {
        return 2;
    }
    else if (rng <= 875)
    {
        return 3;
    }
    else
    {
        return 4;
    }
}

void AddMoveToIndex
(
    const StringId moveName,
    const size_t moveIndex,
    const ecs::World& world,
    Pokemon& pokemon
)
{
    const auto& moveStats = GetMoveStats(moveName, world);

    assert(moveIndex != pokemon.mMoveSet.size() && "Moveset is full");

    pokemon.mMoveSet[moveIndex] = std::make_unique<PokemonMoveStats>
    (
        moveStats.mName,
        moveStats.mType,
        moveStats.mEffect,
        moveStats.mPower,
        moveStats.mAccuracy,
        moveStats.mTotalPowerPoints
    );
}

void AddMoveToFirstUnusedIndex
(
    const StringId moveName,
    const ecs::World& world,
    Pokemon& pokemon
)
{        
    AddMoveToIndex(moveName, FindFirstUnusedMoveIndex(pokemon.mMoveSet), world, pokemon);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

