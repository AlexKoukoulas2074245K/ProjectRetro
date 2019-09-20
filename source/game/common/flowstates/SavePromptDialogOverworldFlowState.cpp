//
//  SavePromptDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 20/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "SavingGameOverworldFlowState.h"
#include "SavePromptDialogOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokedexStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/utils/InputUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 SavePromptDialogOverworldFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(-0.481498629f, -0.065f, -0.4f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

SavePromptDialogOverworldFlowState::SavePromptDialogOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mPlayerStatsTextboxEntityId(CreateSaveScreenPlayerStatsTextbox(mWorld))
{    
    QueueDialogForChatbox(CreateChatbox(mWorld), "Would you like to#SAVE the game?+FREEZE", mWorld);
}

void SavePromptDialogOverworldFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 2 && guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        CreateYesNoTextbox(mWorld, YES_NO_TEXTBOX_POSITION);
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 3)
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
                // Destroy Player Stats textbox
                DestroyGenericOrBareTextbox(mPlayerStatsTextboxEntityId, mWorld);

                CompleteAndTransitionTo<SavingGameOverworldFlowState>();
            }
            // No Selected
            else if (yesNoTextboxCursorRow == 1)
            {
                // Destroy Player Stats textbox
                DestroyGenericOrBareTextbox(mPlayerStatsTextboxEntityId, mWorld);

                // Destroy Main Menu textbox
                DestroyActiveTextbox(mWorld);

                CompleteOverworldFlow();
            }
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);

            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);

            // Destroy Player Stats textbox
            DestroyGenericOrBareTextbox(mPlayerStatsTextboxEntityId, mWorld);

            // Destroy Main Menu textbox
            DestroyActiveTextbox(mWorld);

            CompleteOverworldFlow();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

