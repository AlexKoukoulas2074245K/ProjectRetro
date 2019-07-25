//
//  DamageCalculationEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "DamageCalculationEncounterFlowState.h"
#include "MoveAnnouncementEncounterFlowState.h"
#include "../GameConstants.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/Logging.h"
#include "../utils/PokemonUtils.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

#include <unordered_set>


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

DamageCalculationEncounterFlowState::DamageCalculationEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    const auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    CalculateDamageInternal
    (
        encounterStateComponent.mIsOpponentsTurn ? activeOpponentPokemon : activePlayerPokemon,
        encounterStateComponent.mIsOpponentsTurn ? activePlayerPokemon : activeOpponentPokemon
    );

    if (encounterStateComponent.mIsOpponentsTurn == false)
    {
        encounterStateComponent.mPlayerPokemonToOpponentPokemonDamageMap
        [encounterStateComponent.mActivePlayerPokemonRosterIndex]
        [encounterStateComponent.mActiveOpponentPokemonRosterIndex] += encounterStateComponent.mOutstandingFloatDamage;
    }
}

void DamageCalculationEncounterFlowState::VUpdate(const float)
{        
    CompleteAndTransitionTo<MoveAnnouncementEncounterFlowState>();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void DamageCalculationEncounterFlowState::CalculateDamageInternal
(
    const Pokemon& attackingPokemon,
    const Pokemon& defendingPokemon
) const
{    
    static const std::unordered_set<StringId, StringIdHasher> specialMoves = 
    {
        StringId("WATER"), 
        StringId("GRASS"),
        StringId("FIRE"),
        StringId("ICE"),
        StringId("ELECTRIC"),
        StringId("PSYCHIC"), 
        StringId("DRAGON")
    };

    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& selectedMoveStats = GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld);

    encounterStateComponent.mOutstandingFloatDamage = 0.0f;
    encounterStateComponent.mDefenderFloatHealth = static_cast<float>(defendingPokemon.mHp);

    if (ShouldMoveMiss(selectedMoveStats.mAccuracy, attackingPokemon.mAccuracyStage, defendingPokemon.mEvasionStage))
    {
        encounterStateComponent.mLastMoveMiss = true;        
    }
    else
    {
        encounterStateComponent.mLastMoveCrit = ShouldMoveCrit
        (
            selectedMoveStats.mName,
            attackingPokemon.mSpeedEncounterStage, 
            attackingPokemon.mSpeed
        );

        
        auto isStab = selectedMoveStats.mType == attackingPokemon.mBaseSpeciesStats.mFirstType || selectedMoveStats.mType == attackingPokemon.mBaseSpeciesStats.mSecondType;

        auto effectivenessFactor = GetTypeEffectiveness(selectedMoveStats.mType, defendingPokemon.mBaseSpeciesStats.mFirstType, mWorld);
        if (defendingPokemon.mBaseSpeciesStats.mSecondType != StringId())
        {
            effectivenessFactor *= GetTypeEffectiveness(selectedMoveStats.mType, defendingPokemon.mBaseSpeciesStats.mSecondType, mWorld);
        }

        const auto isSpecialMove = specialMoves.count(selectedMoveStats.mType) != 0;

        const auto effectiveAttackStat  = isSpecialMove ? attackingPokemon.mSpecial : attackingPokemon.mAttack;
        const auto effectiveDefenseStat = isSpecialMove ? defendingPokemon.mSpecial : defendingPokemon.mDefense;

        const auto attackingStatModifier = isSpecialMove ? attackingPokemon.mSpecialEncounterStage : attackingPokemon.mAttackEncounterStage;
        const auto defensiveStatModifier = isSpecialMove ? defendingPokemon.mSpecialEncounterStage : defendingPokemon.mDefenseEncounterStage;

        encounterStateComponent.mOutstandingFloatDamage = static_cast<float>(CalculateDamage
        (
            attackingPokemon.mLevel,
            selectedMoveStats.mPower,
            attackingStatModifier,
            defensiveStatModifier,
            effectiveAttackStat,
            effectiveDefenseStat,
            effectivenessFactor,
            encounterStateComponent.mLastMoveCrit,
            isStab
        ));

        Log(LogType::INFO, "Calculated damage: %.1f", encounterStateComponent.mOutstandingFloatDamage);        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
