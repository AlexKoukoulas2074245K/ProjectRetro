//
//  PCIntroDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PCIntroDialogOverworldFlowState.h"
#include "PCMainOptionsDialogOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PCIntroDialogOverworldFlowState::PC_TURN_ON_SFX_NAME = "general/pc_turn_on";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PCIntroDialogOverworldFlowState::PCIntroDialogOverworldFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    DestroyActiveTextbox(mWorld);
    
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    QueueDialogForChatbox(CreateChatbox(mWorld), playerStateComponent.mPlayerTrainerName.GetString() + " turned on#the PC.#+END", mWorld);
    
    SoundService::GetInstance().PlaySfx(PC_TURN_ON_SFX_NAME);
}

void PCIntroDialogOverworldFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        CompleteAndTransitionTo<PCMainOptionsDialogOverworldFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
