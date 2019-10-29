//
//  PewterCityGymInstructorOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PewterCityGymInstructorOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/NpcAiComponent.h"
#include "../../overworld/utils/OverworldUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PewterCityGymInstructorOverworldFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.065f, -0.4f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PewterCityGymInstructorOverworldFlowState::PewterCityGymInstructorOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{
    DestroyActiveTextbox(mWorld);

    if (HasMilestone(milestones::BOULDERBADGE, mWorld))
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "Just as I thought!#You're POK^MON#champ material!", mWorld);
    }
    else
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "Hiya! I can tell#you have what it#takes to become a#POK^MON champ!#@I'm no trainer,#but I can tell#you how to win!#@Let me take you#to the top!+FREEZE", mWorld);
    }    
}

void PewterCityGymInstructorOverworldFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // Yes/No textbox active
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
        const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
        const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();        
        const auto& npcComponent         = mWorld.GetComponent<NpcAiComponent>(GetNpcEntityIdFromLevelIndex(playerStateComponent.mLastNpcLevelIndexSpokenTo, mWorld));

        const auto yesNoTextboxCursorRow = cursorComponent.mCursorRow;

        if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);

            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);

            // Yes Selected
            if (yesNoTextboxCursorRow == 0)
            {
                const auto mainChatbox = CreateChatbox(mWorld);
                QueueDialogForChatbox(mainChatbox, npcComponent.mSideDialogs[0], mWorld);
            }
            // No Selected
            else if (yesNoTextboxCursorRow == 1)
            {
                const auto mainChatbox = CreateChatbox(mWorld);
                QueueDialogForChatbox(mainChatbox, npcComponent.mSideDialogs[1], mWorld);
            }
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);

            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);

            const auto mainChatbox = CreateChatbox(mWorld);
            QueueDialogForChatbox(mainChatbox, npcComponent.mSideDialogs[1], mWorld);
        }
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 1 && guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        CreateYesNoTextbox(mWorld, YES_NO_TEXTBOX_POSITION);
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        CompleteOverworldFlow();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


