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
#include "FullParalysisTextEncounterFlowState.h"
#include "MoveAnnouncementEncounterFlowState.h"
#include "../GameConstants.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/Logging.h"
#include "../utils/MathUtils.h"
#include "../utils/OSMessageBox.h"
#include "../utils/PokemonUtils.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

#include <cctype>

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
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    if (encounterStateComponent.mAttackingPokemonIsFullyParalyzed)
    {
        CompleteAndTransitionTo<FullParalysisTextEncounterFlowState>();
    }    
    else
    {
        CompleteAndTransitionTo<MoveAnnouncementEncounterFlowState>();
    }    
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
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& selectedMoveStats = GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld);

    encounterStateComponent.mNothingHappendFromMoveExecution = false;
    encounterStateComponent.mOutstandingFloatDamage = 0.0f;
    encounterStateComponent.mDefenderFloatHealth = static_cast<float>(defendingPokemon.mHp);

    encounterStateComponent.mAttackingPokemonIsFullyParalyzed = ShouldAttackingPokemonBeFullyParalyzed(attackingPokemon);
    if (encounterStateComponent.mAttackingPokemonIsFullyParalyzed)
    {
        return;
    }
    else if (ShouldMoveMiss(selectedMoveStats.mAccuracy, attackingPokemon.mAccuracyStage, defendingPokemon.mEvasionStage))
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

        const auto isSpecialMove = isMoveSpecial(selectedMoveStats.mType);

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
    const auto& fullMoveEffectString  = selectedMoveStats.mEffect.GetString();

    encounterStateComponent.mMoveHadNoEffect                           = false;
    encounterStateComponent.mNothingHappendFromMoveExecution           = false;
    encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon   = PokemonStatus::NORMAL;
    encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::NORMAL;

    if (fullMoveEffectString == "ESLP")
    {
        if (encounterStateComponent.mIsOpponentsTurn == true)
        {
            if (defendingPokemon.mStatus == PokemonStatus::NORMAL)
            {
                encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon = PokemonStatus::PARALYZED;
            }
            else
            {
                encounterStateComponent.mMoveHadNoEffect = true;
            }
        }
        else
        {
            if (defendingPokemon.mStatus == PokemonStatus::NORMAL)
            {
                encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::PARALYZED;
            }
            else
            {
                encounterStateComponent.mMoveHadNoEffect = true;
            }
        }
    }
    else if (StringStartsWith(fullMoveEffectString, "EA"))
    {        
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mAttackEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }         
    }
    else if (StringStartsWith(fullMoveEffectString, "PA"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mAttackEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }        
    }
    else if (StringStartsWith(fullMoveEffectString, "ED"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mDefenseEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(fullMoveEffectString, "PD"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mDefenseEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(fullMoveEffectString, "EH"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mAccuracyStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(fullMoveEffectString, "PH"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mAccuracyStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(fullMoveEffectString, "ES"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mSpeedEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(fullMoveEffectString, "PS"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mSpeedEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(fullMoveEffectString, "EX"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mSpecialEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(fullMoveEffectString, "PX"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mSpecialEncounterStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }    
    else if (StringStartsWith(fullMoveEffectString, "EE"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), defendingPokemon.mEvasionStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (StringStartsWith(fullMoveEffectString, "PE"))
    {
        if (!AddToStatStage(std::stoi(selectedMoveStats.mEffect.GetString().substr(2)), attackingPokemon.mEvasionStage))
        {
            encounterStateComponent.mNothingHappendFromMoveExecution = true;
        }
    }
    else if (fullMoveEffectString == "EPAR")
    {
        if (encounterStateComponent.mIsOpponentsTurn == true)
        {
            if (defendingPokemon.mStatus == PokemonStatus::NORMAL)
            {
                encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon = PokemonStatus::PARALYZED;
            }
            else
            {
                encounterStateComponent.mMoveHadNoEffect = true;
            }
        }
        else
        {
            if (defendingPokemon.mStatus == PokemonStatus::NORMAL)
            {
                encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::PARALYZED;
            }
            else
            {
                encounterStateComponent.mMoveHadNoEffect = true;
            }
        }
    }
    else if (fullMoveEffectString == "EPOI")
    {
        if (encounterStateComponent.mIsOpponentsTurn == true)
        {
            if (defendingPokemon.mStatus == PokemonStatus::NORMAL)
            {
                encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon = PokemonStatus::POISONED;
            }
            else
            {
                encounterStateComponent.mMoveHadNoEffect = true;
            }
        }
        else
        {
            if (defendingPokemon.mStatus == PokemonStatus::NORMAL)
            {
                encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::POISONED;
            }
            else
            {
                encounterStateComponent.mMoveHadNoEffect = true;
            }
        }
    }
    else if (std::isdigit(fullMoveEffectString[0]))
    {
        const auto probability = std::stoi(fullMoveEffectString.substr(0, 2));
        const auto shouldProc  = math::RandomInt(0, 99) <= probability;    
        
        if (shouldProc)
        {            
            const auto& moveEffectName = fullMoveEffectString.substr(3);

            if (moveEffectName == "EPAR")
            {
                if (defendingPokemon.mStatus == PokemonStatus::NORMAL && DoesPokemonHaveType(PokemonType::ELECTRIC, defendingPokemon) == false)
                {
                    if (encounterStateComponent.mIsOpponentsTurn)
                    {
                        encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon = PokemonStatus::PARALYZED;
                    }
                    else
                    {
                        encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::PARALYZED;
                    }
                }
            }       
            else if (moveEffectName == "EPOI")
            {
                if (defendingPokemon.mStatus == PokemonStatus::NORMAL)
                {
                    if (encounterStateComponent.mIsOpponentsTurn)
                    {
                        encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon = PokemonStatus::POISONED;
                    }
                    else
                    {
                        encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::POISONED;
                    }
                }
            }
            else if (moveEffectName == "ECON")
            {
                if (defendingPokemon.mStatus != PokemonStatus::CONFUSED)
                {
                    if (encounterStateComponent.mIsOpponentsTurn)
                    {
                        encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon = PokemonStatus::CONFUSED;
                        defendingPokemon.mNumberOfRoundsUntilConfusionEnds               = math::RandomInt(1, 1);
                    }
                    else
                    {
                        encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::CONFUSED;
                        defendingPokemon.mNumberOfRoundsUntilConfusionEnds                 = math::RandomInt(1, 1);
                    }
                }
            }
        }
    }
    else
    {
        ShowMessageBox(MessageBoxType::INFO, "Move Effect Not Handled", "Move effect: " + fullMoveEffectString + " not handled");
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
