//
//  BindOrWrapBlockedAttackTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BindOrWrapBlockedAttackTextEncounterFlowState.h"
#include "TurnOverEncounterFlowState.h"
#include "../utils/TextboxUtils.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

BindOrWrapBlockedAttackTextEncounterFlowState::BindOrWrapBlockedAttackTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();            
    const auto& activePlayerPokemon     = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon   = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
  
    const auto mainChatboxEntityId = CreateChatbox(world);

    if (encounterStateComponent.mIsOpponentsTurn)
    {
        QueueDialogForChatbox(mainChatboxEntityId, "Enemy " + activeOpponentPokemon.mName.GetString() + "#can't move!#+END", mWorld);
    }
    else
    {
        QueueDialogForChatbox(mainChatboxEntityId, activePlayerPokemon.mName.GetString() + "#can't move!#+END", mWorld);
    }   
}

void BindOrWrapBlockedAttackTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        CompleteAndTransitionTo<TurnOverEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
