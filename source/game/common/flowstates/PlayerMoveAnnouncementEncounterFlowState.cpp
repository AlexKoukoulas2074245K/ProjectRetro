//
//  PlayerMoveAnnouncementEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PlayerMoveAnnouncementEncounterFlowState.h"
#include "PlayerMoveAnimationEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerMoveAnnouncementEncounterFlowState::PlayerMoveAnnouncementEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto mainChatboxEntityId = CreateChatbox(world);

    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activePlayerPokemon     = *playerStateComponent.mPlayerPokemonRoster.front();
    const auto& selectedMove            = *activePlayerPokemon.mMoveSet[encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu];

    //TODO: differentiate between summoning dialogs
    QueueDialogForTextbox
    (
        mainChatboxEntityId,
        activePlayerPokemon.mName.GetString() + "#used " +  selectedMove.mName.GetString() + "!+FREEZE",
        mWorld
    );
}

void PlayerMoveAnnouncementEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        CompleteAndTransitionTo<PlayerMoveAnimationEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

