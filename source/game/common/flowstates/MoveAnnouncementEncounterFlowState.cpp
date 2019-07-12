//
//  MoveAnnouncementEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MoveAnnouncementEncounterFlowState.h"
#include "MoveAnimationEncounterFlowState.h"
#include "MoveMissEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MoveAnnouncementEncounterFlowState::MoveAnnouncementEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto mainChatboxEntityId = CreateChatbox(world);

    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();    
    const auto& selectedMove            = encounterStateComponent.mLastMoveSelected;

    const auto& attackingPokemon = encounterStateComponent.mIsOpponentsTurn ?
        encounterStateComponent.mOpponentPokemonRoster.front() :
        playerStateComponent.mPlayerPokemonRoster.front();


    //TODO: differentiate between summoning dialogs
    QueueDialogForTextbox
    (
        mainChatboxEntityId,        
        attackingPokemon->mName.GetString() + "#used " +  selectedMove.GetString() + "!+FREEZE",
        mWorld
    );
}

void MoveAnnouncementEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        if (encounterStateComponent.mLastMoveMiss)
        {
            CompleteAndTransitionTo<MoveMissEncounterFlowState>();
        }
        else
        {
            CompleteAndTransitionTo<MoveAnimationEncounterFlowState>();
        }        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

