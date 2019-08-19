//
//  FullParalysisTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "FullParalysisTextEncounterFlowState.h"
#include "PoisonTickCheckEncounterFlowState.h"
#include "TurnOverEncounterFlowState.h"
#include "../utils/TextboxUtils.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

FullParalysisTextEncounterFlowState::FullParalysisTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();            
    const auto& activePlayerPokemon     = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon   = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    
    const auto mainChatboxEntityId = CreateChatbox(world);
    
    if (encounterStateComponent.mIsOpponentsTurn)
    {
        QueueDialogForChatbox(mainChatboxEntityId, "Enemy " + activeOpponentPokemon.mName.GetString() + "'s#fully paralyzed!#+END", mWorld);
    }
    else
    {
        QueueDialogForChatbox(mainChatboxEntityId, activePlayerPokemon.mName.GetString() + "'s#fully paralyzed!#+END", mWorld);
    }
}

void FullParalysisTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        CompleteAndTransitionTo<PoisonTickCheckEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
