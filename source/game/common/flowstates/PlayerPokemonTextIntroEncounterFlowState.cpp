//
//  PlayerPokemonTextIntroEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PlayerPokemonTextIntroEncounterFlowState.h"
#include "PlayerPokemonSummonEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerPokemonTextIntroEncounterFlowState::PlayerPokemonTextIntroEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto overlaidChatboxEntityId = CreateChatbox(mWorld);

    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    //TODO: differentiate between summoning dialogs
    QueueDialogForTextbox(overlaidChatboxEntityId, "GO! " + playerStateComponent.mPlayerPokemonRoster.front().mName.GetString() + "!+FREEZE", mWorld);
}

void PlayerPokemonTextIntroEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        CompleteAndTransitionTo<PlayerPokemonSummonEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

