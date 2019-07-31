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
#include "../utils/OSMessageBox.h"
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

    auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

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
    Pokemon& attackingPokemon,
    Pokemon& defendingPokemon
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
        encounterStateComponent.mLastMoveMiss = false;
        encounterStateComponent.mLastMoveCrit = ShouldMoveCrit
        (
            selectedMoveStats.mName,            
            attackingPokemon.mBaseSpeciesStats.mSpeed
        );
        
        auto isStab = selectedMoveStats.mType == attackingPokemon.mBaseSpeciesStats.mFirstType || selectedMoveStats.mType == attackingPokemon.mBaseSpeciesStats.mSecondType;

        auto effectivenessFactor = GetTypeEffectiveness(selectedMoveStats.mType, defendingPokemon.mBaseSpeciesStats.mFirstType, mWorld);
        if (defendingPokemon.mBaseSpeciesStats.mSecondType != StringId())
        {
            effectivenessFactor *= GetTypeEffectiveness(selectedMoveStats.mType, defendingPokemon.mBaseSpeciesStats.mSecondType, mWorld);
        }

        if (effectivenessFactor <= 0.1f)
        {
            return;
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

        if (selectedMoveStats.mEffect != StringId())
        {
            HandleMoveEffect(attackingPokemon, defendingPokemon);
        }

        Log(LogType::INFO, "Calculated damage: %.1f", encounterStateComponent.mOutstandingFloatDamage);        
    }
}

void DamageCalculationEncounterFlowState::HandleMoveEffect
(    
    Pokemon& attackingPokemon,
    Pokemon& defendingPokemon
) const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& selectedMoveStats = GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld);
    const auto& moveEffectString  = selectedMoveStats.mEffect.GetString();

    encounterStateComponent.mNothingHappendFromMoveExecution = false;

    if (StringStartsWith(moveEffectString, "EA"))
    {        
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mAttackEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }         
    }
    else if (StringStartsWith(moveEffectString, "PA"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mAttackEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }        
    }
    else if (StringStartsWith(moveEffectString, "ED"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mDefenseEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(moveEffectString, "PD"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mDefenseEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(moveEffectString, "EH"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mAccuracyStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(moveEffectString, "PH"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mAccuracyStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(moveEffectString, "ES"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mSpeedEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(moveEffectString, "PS"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mSpeedEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(moveEffectString, "EX"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mSpecialEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(moveEffectString, "PX"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mSpecialEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }    
    else if (StringStartsWith(moveEffectString, "EE"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mEvasionStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(moveEffectString, "PE"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mEvasionStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else
    {
        ShowMessageBox(MessageBoxType::INFO, "Move Effect Not Handled", "Move effect: " + moveEffectString + " not handled");
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
