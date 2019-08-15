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

    if (encounterStateComponent.mIsOpponentsTurn && encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon == PokemonStatus::CONFUSED)
    {
        activeOpponentPokemon.mStatus = encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon;
        encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::NORMAL;
        CompleteAndTransitionTo<PokemonConfusedTextEncounterFlowState>();
    }
    else if (encounterStateComponent.mIsOpponentsTurn && activeOpponentPokemon.mStatus == PokemonStatus::CONFUSED)
    {
        CompleteAndTransitionTo<PokemonConfusedTextEncounterFlowState>();
    }
    else if (encounterStateComponent.mIsOpponentsTurn == false && encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon == PokemonStatus::CONFUSED)
    {
        activePlayerPokemon.mStatus = encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon;
        encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon = PokemonStatus::NORMAL;
        CompleteAndTransitionTo<PokemonConfusedTextEncounterFlowState>();
    }
    else if (encounterStateComponent.mIsOpponentsTurn == false && activePlayerPokemon.mStatus == PokemonStatus::CONFUSED)
    {
        CompleteAndTransitionTo<PokemonConfusedTextEncounterFlowState>();
    }
    else
    {
        CompleteAndTransitionTo<DamageCalculationEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

