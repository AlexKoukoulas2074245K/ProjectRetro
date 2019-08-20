//
//  PreDamageCalculationChecksEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 15/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PreDamageCalculationChecksEncounterFlowState.h"
#include "DamageCalculationEncounterFlowState.h"
#include "PokemonConfusedTextEncounterFlowState.h"
#include "PokemonSnappedOutOfConfusionEncounterFlowState.h"
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
    else
    {
        CompleteAndTransitionTo<DamageCalculationEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
