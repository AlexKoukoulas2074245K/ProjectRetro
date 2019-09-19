//
//  ViridianSchoolBookOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 19/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ViridianSchoolBookOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 ViridianSchoolBookOverworldFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.065f, -0.4f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ViridianSchoolBookOverworldFlowState::ViridianSchoolBookOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{            
    DestroyActiveTextbox(mWorld);    
    
    mBookDialog.push("Second page... # #@A healthy POK^MON#may be hard to#catch, so weaken#it first!#@Poison, burns and#other damage are#effective!#+END");
    mBookDialog.push("Third page... # #@POK^MON trainers#seek others to#engage in POK^MON#fights.#@Battles are#constantly fought#at POK^MON GYMs.#+END");
    mBookDialog.push("Fourth page... # #@The goal for#POK^MON trainers#is to beat the#top 8 POK^MON#GYM LEADERs.#@Do so to earn the#right to face...#@The ELITE FOUR of#POK^MON LEAGUE!#@GIRL: Hey! Don't#look at my notes!");

    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox(mainChatboxEntityId, "Turn the page?+FREEZE", mWorld);
}

void ViridianSchoolBookOverworldFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        if (guiStateComponent.mActiveTextboxesStack.size() == 1)
        {
            CreateYesNoTextbox(mWorld, YES_NO_TEXTBOX_POSITION);
        }
        else
        {
            const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
            const auto& cursorComponent = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));            

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
                    QueueDialogForChatbox(mainChatbox, mBookDialog.front(), mWorld);
                    mBookDialog.pop();
                }
                // No Selected
                else if (yesNoTextboxCursorRow == 1)
                {
                    CompleteOverworldFlow();
                }
            }
            else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
            {
                // Destroy Yes/No textbox
                DestroyActiveTextbox(mWorld);

                // Destroy main chatbox
                DestroyActiveTextbox(mWorld);

                CompleteOverworldFlow();
            }
        }
    }
    else
    {
        // Book dialog end
        if (guiStateComponent.mActiveTextboxesStack.size() == 0)
        {
            if (mBookDialog.size() > 0)
            {
                const auto mainChatboxEntityId = CreateChatbox(mWorld);
                QueueDialogForChatbox(mainChatboxEntityId, "Turn the page?+FREEZE", mWorld);
            }
            else
            {
                CompleteOverworldFlow();
            }
        }
    }

}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


