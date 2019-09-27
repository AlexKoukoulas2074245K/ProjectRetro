//
//  PokeMartMenuSelectionOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 26/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokeMartMenuSelectionOverworldFlowState.h"
#include "PokeMartBuyDialogOverworldFlowState.h"
#include "PokeMartSellDialogOverworldFlowState.h"
#include "PokeMartFarewellDialogOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokeMartDialogStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PokeMartMenuSelectionOverworldFlowState::TEXTBOX_CLICK_SFX_NAME = "general/textbox_click";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokeMartMenuSelectionOverworldFlowState::PokeMartMenuSelectionOverworldFlowState(ecs::World& world)
    : BaseFlowState(world)
{   
    DisplayMenuTextbox();
    DisplayMoneyTextbox();
}

void PokeMartMenuSelectionOverworldFlowState::VUpdate(const float)
{        
    const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
    const auto menuCursorRow         = cursorComponent.mCursorRow;
    
    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        // Buy
        if (menuCursorRow == 0)
        {
            CompleteAndTransitionTo<PokeMartBuyDialogOverworldFlowState>();
        }
        // Sell
        else if (menuCursorRow == 1)
        {
            CompleteAndTransitionTo<PokeMartSellDialogOverworldFlowState>();
        }
        // Quit
        else
        {
            CompleteAndTransitionTo<PokeMartFarewellDialogOverworldFlowState>();
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        SoundService::GetInstance().PlaySfx(TEXTBOX_CLICK_SFX_NAME);
        CompleteAndTransitionTo<PokeMartFarewellDialogOverworldFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PokeMartMenuSelectionOverworldFlowState::DisplayMenuTextbox() const
{
    auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
    pokeMartDialogStateComponent.mMenuTextboxEntityId = CreatePokeMartMenuTextbox(mWorld);
}

void PokeMartMenuSelectionOverworldFlowState::DisplayMoneyTextbox() const
{    
    auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
    pokeMartDialogStateComponent.mMoneyTextboxEntityId = CreatePokeMartMoneyTextbox(mWorld);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

