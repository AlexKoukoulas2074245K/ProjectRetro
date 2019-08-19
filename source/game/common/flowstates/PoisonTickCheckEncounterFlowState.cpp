//
//  PoisonTickCheckEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 19/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PoisonTickAnimationEncounterFlowState.h"
#include "PoisonTickCheckEncounterFlowState.h"
#include "TurnOverEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PoisonTickCheckEncounterFlowState::PoisonTickCheckEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{        
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    auto& attackingPokemon =
        encounterStateComponent.mIsOpponentsTurn ?
        *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex] :
        *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
        
        
    if (attackingPokemon.mStatus == PokemonStatus::POISONED)
    {
        auto poisonTickText = attackingPokemon.mName.GetString() + "'s#hurt by poison!#+END";
        if (encounterStateComponent.mIsOpponentsTurn)
        {
            poisonTickText = "Enemy " + poisonTickText;
        }        
        
        const auto mainChatboxEntityId = CreateChatbox(mWorld);
        QueueDialogForChatbox(mainChatboxEntityId, poisonTickText, mWorld);
    }
}

void PoisonTickCheckEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
        auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

        auto& attackingPokemon =
            encounterStateComponent.mIsOpponentsTurn ?
            *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex] :
            *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];

        if (attackingPokemon.mStatus == PokemonStatus::POISONED)
        {
            CompleteAndTransitionTo<PoisonTickAnimationEncounterFlowState>();
        }
        else
        { 
            CompleteAndTransitionTo<TurnOverEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
