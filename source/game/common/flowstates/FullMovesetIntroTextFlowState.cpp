//
//  FullMovesetIntroTextFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 02/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "FullMovesetIntroTextFlowState.h"
#include "AbandonLearningNewMoveFlowState.h"
#include "SelectWhichMoveToReplaceFlowState.h"
#include "../../common/components/CursorComponent.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 FullMovesetIntroTextFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.058000f, -0.1f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

FullMovesetIntroTextFlowState::FullMovesetIntroTextFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activePlayerPokemon     = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];
    
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        DestroyActiveTextbox(mWorld);
    }
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        activePlayerPokemon.mName.GetString() + " is#trying to learn#" +
        activePlayerPokemon.mMoveToBeLearned.GetString() + "!#@But, " +
        activePlayerPokemon.mName.GetString() + "#can't learn more#than 4 moves!#@" +
        "Delete an older#move to make room#for " + activePlayerPokemon.mMoveToBeLearned.GetString() + "?+FREEZE",
        mWorld
    );
}

void FullMovesetIntroTextFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        const auto activeTextboxEntityId = GetActiveTextboxEntityId(mWorld);
        const auto& activeTextboxComponent = mWorld.GetComponent<TextboxComponent>(activeTextboxEntityId);
        
        // Yes-No textbox has already been created
        if (activeTextboxComponent.mTextboxType == TextboxType::CURSORED_TEXTBOX)
        {
            const auto& cursorComponent     = mWorld.GetComponent<CursorComponent>(activeTextboxEntityId);
            const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
            
            if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
            {
                // Yes selected
                if (cursorComponent.mCursorRow == 0)
                {
                    TransitionToSelectWhichMoveToReplace();
                }
                // No selected
                else
                {
                    TransitionToAbandonLearningNewMoveQuestion();
                }
            }
            else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
            {
                TransitionToAbandonLearningNewMoveQuestion();
            }
        }
        else
        {
            CreateYesNoTextbox(mWorld, glm::vec3(YES_NO_TEXTBOX_POSITION.x, YES_NO_TEXTBOX_POSITION.y, YES_NO_TEXTBOX_POSITION.z));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void FullMovesetIntroTextFlowState::TransitionToSelectWhichMoveToReplace()
{
    // Destroy Yes/No textbox
    DestroyActiveTextbox(mWorld);
    
    // Destroy Chatbox
    DestroyActiveTextbox(mWorld);
    
    CompleteAndTransitionTo<SelectWhichMoveToReplaceFlowState>();
}

void FullMovesetIntroTextFlowState::TransitionToAbandonLearningNewMoveQuestion()
{
    // Destroy Yes/No textbox
    DestroyActiveTextbox(mWorld);
    
    // Destroy Chatbox
    DestroyActiveTextbox(mWorld);
    
    CompleteAndTransitionTo<AbandonLearningNewMoveFlowState>();
}
