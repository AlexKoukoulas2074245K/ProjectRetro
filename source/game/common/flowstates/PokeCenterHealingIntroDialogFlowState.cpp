//
//  PokeCenterHealingIntroDialogFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokeCenterHealingAnimationFlowState.h"
#include "PokeCenterHealingFarewellDialogFlowState.h"
#include "PokeCenterHealingIntroDialogFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PokeCenterHealingIntroDialogFlowState::HEAL_CANCEL_TEXTBOX_POSITION = glm::vec3(0.378, -0.01f, -0.4f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokeCenterHealingIntroDialogFlowState::PokeCenterHealingIntroDialogFlowState(ecs::World& world)
    : BaseFlowState(world)
{   
    CreateHealCancelTextbox(mWorld, HEAL_CANCEL_TEXTBOX_POSITION);
}

void PokeCenterHealingIntroDialogFlowState::VUpdate(const float)
{    
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // Heal/Cancel flow
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
        const auto& cursorComponent = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));

        const auto healCancelTextboxCursorRow = cursorComponent.mCursorRow;

        if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
        {
            // Destroy Heal/Cancel textbox
            DestroyActiveTextbox(mWorld);

            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);

            // Heal Selected
            if (healCancelTextboxCursorRow == 0)
            {
                const auto mainChatboxEntityId = CreateChatbox(mWorld);
                QueueDialogForChatbox(mainChatboxEntityId, "OK. We'll need#your POKEMON.+FREEZE", mWorld);
            }
            // Cancel Selected
            else
            {
                CompleteAndTransitionTo<PokeCenterHealingFarewellDialogFlowState>();
            }
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            // Destroy Heal/Cancel textbox
            DestroyActiveTextbox(mWorld);

            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);

            CompleteAndTransitionTo<PokeCenterHealingFarewellDialogFlowState>();
        }
    }
    // Heal intro text flow
    else if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        CompleteAndTransitionTo<PokeCenterHealingAnimationFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


