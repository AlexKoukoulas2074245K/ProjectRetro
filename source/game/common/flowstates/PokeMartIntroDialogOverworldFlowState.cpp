//
//  PokeMartIntroDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 26/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokeMartIntroDialogOverworldFlowState.h"
#include "PokeMartMenuSelectionOverworldFlowState.h"
#include "../components/DirectionComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/utils/AnimationUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const int PokeMartIntroDialogOverworldFlowState::NPC_BEHIND_COUNTER_LEVEL_INDEX = 6;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokeMartIntroDialogOverworldFlowState::PokeMartIntroDialogOverworldFlowState(ecs::World& world)
    : BaseFlowState(world)
{   
    DestroyActiveTextbox(mWorld);

    // Change direction of npc behind counter
    auto& npcRenderableComponent   = mWorld.GetComponent<RenderableComponent>(GetNpcEntityIdFromLevelIndex(NPC_BEHIND_COUNTER_LEVEL_INDEX, mWorld));
    auto& playerDirectionComponent = mWorld.GetComponent<DirectionComponent>(GetPlayerEntityId(mWorld));

    const auto directionFacingPlayer = static_cast<Direction>((static_cast<int>(playerDirectionComponent.mDirection) + 2) % 4);
    ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(directionFacingPlayer), npcRenderableComponent);

    QueueDialogForChatbox(CreateChatbox(mWorld), "Hi there!#May I help you?+FREEZE", mWorld);
}

void PokeMartIntroDialogOverworldFlowState::VUpdate(const float)
{    
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        CompleteAndTransitionTo<PokeMartMenuSelectionOverworldFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


