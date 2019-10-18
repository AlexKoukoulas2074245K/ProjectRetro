//
//  PreDamageCalculationChecksEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 15/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BideUnleashTextEncounterFlowState.h"
#include "BindWrapContinuationEncounterFlowState.h"
#include "DamageCalculationEncounterFlowState.h"
#include "PokemonConfusedTextEncounterFlowState.h"
#include "PokemonFastAsleepTextEncounterFlowState.h"
#include "PokemonJustWokeUpEncounterFlowState.h"
#include "PokemonSnappedOutOfConfusionEncounterFlowState.h"
#include "PreDamageCalculationChecksEncounterFlowState.h"
#include "TurnOverEncounterFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PreDamageCalculationChecksEncounterFlowState::PreDamageCalculationChecksEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
}

void PreDamageCalculationChecksEncounterFlowState::VUpdate(const float)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    if
    (
        encounterStateComponent.mIsOpponentsTurn &&
        (
            activeOpponentPokemon.mNumberOfRoundsUntilConfusionEnds > 0 ||
            activeOpponentPokemon.mStatus == PokemonStatus::CONFUSED
        )
    )
    {
        if (--activeOpponentPokemon.mNumberOfRoundsUntilConfusionEnds < 0)
        {
            CompleteAndTransitionTo<PokemonSnappedOutOfConfusionEncounterFlowState>();
        }
        else
        {
            if (encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon == PokemonStatus::CONFUSED)
            {
                encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::NORMAL;
                activeOpponentPokemon.mStatus = PokemonStatus::CONFUSED;
            }

            CompleteAndTransitionTo<PokemonConfusedTextEncounterFlowState>();
        }                                
    }    
    else if
    (
        encounterStateComponent.mIsOpponentsTurn == false &&
        (
            activePlayerPokemon.mNumberOfRoundsUntilConfusionEnds > 0 ||
            activePlayerPokemon.mStatus == PokemonStatus::CONFUSED
        )
    )
    {
        if (--activePlayerPokemon.mNumberOfRoundsUntilConfusionEnds < 0)
        {
            CompleteAndTransitionTo<PokemonSnappedOutOfConfusionEncounterFlowState>();
        }
        else
        {
            if (encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon == PokemonStatus::CONFUSED)
            {
                encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon = PokemonStatus::NORMAL;
                activePlayerPokemon.mStatus = PokemonStatus::CONFUSED;
            }

            CompleteAndTransitionTo<PokemonConfusedTextEncounterFlowState>();
        }        
    }
    else if
    (
        encounterStateComponent.mIsOpponentsTurn == false &&
        (
            activePlayerPokemon.mNumberOfRoundsUntilSleepEnds > 0 ||
            activePlayerPokemon.mStatus == PokemonStatus::ASLEEP
        )
    )
    {
        if (--activePlayerPokemon.mNumberOfRoundsUntilSleepEnds < 0)
        {
            CompleteAndTransitionTo<PokemonJustWokeUpEncounterFlowState>();
        }
        else
        {
            if (encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon == PokemonStatus::ASLEEP)
            {
                encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon = PokemonStatus::NORMAL;
                activePlayerPokemon.mStatus = PokemonStatus::ASLEEP;
            }
            
            CompleteAndTransitionTo<PokemonFastAsleepTextEncounterFlowState>();
        }
    }
    else if
    (
        encounterStateComponent.mIsOpponentsTurn &&
        (
            activeOpponentPokemon.mNumberOfRoundsUntilSleepEnds > 0 ||
            activeOpponentPokemon.mStatus == PokemonStatus::ASLEEP
        )
    )
    {
        if (--activeOpponentPokemon.mNumberOfRoundsUntilSleepEnds < 0)
        {
            CompleteAndTransitionTo<PokemonJustWokeUpEncounterFlowState>();
        }
        else
        {
            if (encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon == PokemonStatus::ASLEEP)
            {
                encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::NORMAL;
                activeOpponentPokemon.mStatus = PokemonStatus::ASLEEP;
            }
            
            CompleteAndTransitionTo<PokemonFastAsleepTextEncounterFlowState>();
        }
    }
    else if 
    (
        encounterStateComponent.mIsOpponentsTurn && 
        activeOpponentPokemon.mBideCounter > -1
    )
    {
        if (--activeOpponentPokemon.mBideCounter == -1)
        {
            CompleteAndTransitionTo<BideUnleashTextEncounterFlowState>();
        }
        else
        {
            encounterStateComponent.mIsOpponentsTurn = true;
            CompleteAndTransitionTo<TurnOverEncounterFlowState>();
        }        
    }
    else if
    (
        encounterStateComponent.mIsOpponentsTurn == false &&
        activePlayerPokemon.mBideCounter > -1
    )
    {
        if (--activePlayerPokemon.mBideCounter == -1)
        {
            CompleteAndTransitionTo<BideUnleashTextEncounterFlowState>();
        }
        else
        {
            encounterStateComponent.mIsOpponentsTurn = false;
            CompleteAndTransitionTo<TurnOverEncounterFlowState>();
        }        
    }
    else if 
    (
        activePlayerPokemon.mBindingOrWrappingOpponentCounter > -1  && 
        encounterStateComponent.mBindOrWrapState == BindOrWrapState::CONTINUATION
    )
    {
        activePlayerPokemon.mBindingOrWrappingOpponentCounter--;
        CompleteAndTransitionTo<BindWrapContinuationEncounterFlowState>();
    }
    else if
    (
        activeOpponentPokemon.mBindingOrWrappingOpponentCounter > -1 &&
        encounterStateComponent.mBindOrWrapState == BindOrWrapState::CONTINUATION
    )
    {
        activeOpponentPokemon.mBindingOrWrappingOpponentCounter--;
        CompleteAndTransitionTo<BindWrapContinuationEncounterFlowState>();      
    }
    else
    {        
        CompleteAndTransitionTo<DamageCalculationEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

