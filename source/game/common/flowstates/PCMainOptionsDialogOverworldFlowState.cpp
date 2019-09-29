//
//  PCMainOptionsDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PCMainOptionsDialogOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PCMainOptionsDialogOverworldFlowState::PC_TURN_OFF_SFX_NAME = "general/pc_turn_off";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PCMainOptionsDialogOverworldFlowState::PCMainOptionsDialogOverworldFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    CreatePCMainOptionsTextbox(mWorld, true);
}

void PCMainOptionsDialogOverworldFlowState::VUpdate(const float)
{
    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
