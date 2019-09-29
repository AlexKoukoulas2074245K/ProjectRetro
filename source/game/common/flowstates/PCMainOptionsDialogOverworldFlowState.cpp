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
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/utils/InputUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PCMainOptionsDialogOverworldFlowState::PC_TURN_OFF_SFX_NAME = "general/pc_turn_off";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PCMainOptionsDialogOverworldFlowState::PCMainOptionsDialogOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{
    CreatePCMainOptionsTextbox(mWorld, true);
}

void PCMainOptionsDialogOverworldFlowState::VUpdate(const float)
{
    const auto& inputStateComponent    = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& cursorComponent        = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
    const auto menuCursorRow           = cursorComponent.mCursorRow;
    
    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        // Pokemon PC
        if (menuCursorRow == 0)
        {
            
        }
        // Prof Oak's PC
        else if (menuCursorRow == 1)
        {
            
        }
        // Log off
        else
        {
            SoundService::GetInstance().PlaySfx(PC_TURN_OFF_SFX_NAME);
            DestroyActiveTextbox(mWorld);
            CompleteOverworldFlow();
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        SoundService::GetInstance().PlaySfx(PC_TURN_OFF_SFX_NAME);
        DestroyActiveTextbox(mWorld);
        CompleteOverworldFlow();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
