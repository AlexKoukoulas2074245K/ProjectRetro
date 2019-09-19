//
//  ViridianSchoolBlackboardOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 19/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ViridianSchoolBlackboardOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ViridianSchoolBlackboardOverworldFlowState::ViridianSchoolBlackboardOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{            
    const auto mainChatboxEntityId = GetActiveTextboxEntityId(mWorld);
    WriteTextAtTextboxCoords(mainChatboxEntityId, "Which heading do", 1, 2, mWorld);
    WriteTextAtTextboxCoords(mainChatboxEntityId, "you want to read?", 1, 4, mWorld);

    CreateBlackboardTextbox(mWorld);    
}

void ViridianSchoolBlackboardOverworldFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // Blackboard and chatbox active
    if (guiStateComponent.mActiveTextboxesStack.size() == 2 && mWorld.HasComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld)))
    {
        const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
        const auto& cursorComponent     = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));                

        if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
        {            
            const auto blackboardCursorCol = cursorComponent.mCursorCol;
            const auto blackboardCursorRow = cursorComponent.mCursorRow;

            // Quit Selected
            if (blackboardCursorCol == 1 && blackboardCursorRow == 2)
            {
                // Destroy blackboard textbox
                DestroyActiveTextbox(mWorld);

                // Destroy main chatbox
                DestroyActiveTextbox(mWorld);

                CompleteOverworldFlow();
            }
            // Status selected
            else
            {
                ShowBlackboardExplanationText(blackboardCursorCol, blackboardCursorRow);
            }
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            // Destroy blackboard textbox
            DestroyActiveTextbox(mWorld);

            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);

            CompleteOverworldFlow();
        }
    }
    // Status dialog just finished
    else if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        const auto& blackboardCursorComponent = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));

        const auto currentCursorCol = blackboardCursorComponent.mCursorCol;
        const auto currentCursorRow = blackboardCursorComponent.mCursorRow;

        // Destroy blackboard textbox
        DestroyActiveTextbox(mWorld);

        // Create heading selection chatbox
        const auto mainChatboxEntityId = CreateChatbox(mWorld);

        WriteTextAtTextboxCoords(mainChatboxEntityId, "Which heading do", 1, 2, mWorld);
        WriteTextAtTextboxCoords(mainChatboxEntityId, "you want to read?", 1, 4, mWorld);

        // Recreate blackboard textbox
        CreateBlackboardTextbox(mWorld, currentCursorCol, currentCursorRow);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void ViridianSchoolBlackboardOverworldFlowState::ShowBlackboardExplanationText
(
    const int blackboardCursorCol, 
    const int blackboardCursorRow
) const
{
    // Destroy blackboard textbox
    DestroyActiveTextbox(mWorld);

    // Destroy main chatbox
    DestroyActiveTextbox(mWorld);

    // Recreate blackboard textbox
    const auto blackboardTextboxEntityId = CreateBlackboardTextbox(mWorld, blackboardCursorCol, blackboardCursorRow);

    // Create status explanation chatbox
    const auto statusExplanationChatboxEntityId = CreateChatbox(mWorld);

    auto statusText = std::string();

    // SLP selected
    if (blackboardCursorCol == 0 && blackboardCursorRow == 0)
    {
        statusText = "A POK^MON can't#attack if it's#asleep!#@POK^MON will stay#asleep even after#battles.#@Use AWAKENING to#wake them up!#+END";
    }
    // BRN selected
    else if (blackboardCursorCol == 1 && blackboardCursorRow == 0)
    {
        statusText = "A burn reduces#power and speed.#It also causes#ongoing damage.#@Burns remain#after battles.#@Use BURN HEAL to#cure a burn!#+END";
    }
    // PSN selected
    else if (blackboardCursorCol == 0 && blackboardCursorRow == 1)
    {
        statusText = "When poisoned, a#POK^MON's health#steadily drops.#@Poison lingers#after battles.#@Use an ANTIDOTE#to cure poison!#+END";
    }
    // FRZ selected
    else if (blackboardCursorCol == 1 && blackboardCursorRow == 1)
    {
        statusText = "If frozen, a#POK^MON becomes#totally immobile!#@It stays frozen#even after the#battle ends.#@Use ICE HEAL to#thaw out POK^MON!#+END";
    }
    // PAR selected
    else if (blackboardCursorCol == 0 && blackboardCursorRow == 2)
    {
        statusText = "Paralysis could#make POK^MON#moves misfire!#@Paralysis remains#after battles.#@Use PARLYZ HEAL#for treatment!#+END";
    }

    QueueDialogForChatbox(statusExplanationChatboxEntityId, statusText, mWorld);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
