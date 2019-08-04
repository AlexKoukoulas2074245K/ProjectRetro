//
//  SelectWhichMoveToReplaceFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 04/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "AbandonLearningNewMoveFlowState.h"
#include "LearnNewMoveFlowState.h"
#include "SelectWhichMoveToReplaceFlowState.h"
#include "../../common/components/CursorComponent.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/utils/PokemonUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 SelectWhichMoveToReplaceFlowState::MOVES_TEXTBOX_POSITION = glm::vec3(0.134f, -0.11f, -0.1f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

SelectWhichMoveToReplaceFlowState::SelectWhichMoveToReplaceFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        "Which move should#be forgotten?#+FREEZE",
        mWorld
    );
}

void SelectWhichMoveToReplaceFlowState::VUpdate(const float)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        CompleteAndTransitionTo<LearnNewMoveFlowState>();
    }
    else if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        const auto activeTextboxEntityId = GetActiveTextboxEntityId(mWorld);
        const auto& activeTextboxComponent = mWorld.GetComponent<TextboxComponent>(activeTextboxEntityId);
        
        // Move selection textbox has already been created
        if (activeTextboxComponent.mTextboxType == TextboxType::CURSORED_TEXTBOX)
        {
            const auto& cursorComponent     = mWorld.GetComponent<CursorComponent>(activeTextboxEntityId);
            const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
            
            if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
            {
                const auto cursorRow = cursorComponent.mCursorRow;
                
                // Destroy Move Selection Textbox
                DestroyActiveTextbox(mWorld);
                
                // Destroy Chatbox
                DestroyActiveTextbox(mWorld);
                
                const auto mainChatboxEntityId = CreateChatbox(mWorld);
                QueueDialogForChatbox
                (
                    mainChatboxEntityId,
                    "1, 2 and... Poof!# #@" + activePlayerPokemon.mName.GetString() +
                    " forgot#" + activePlayerPokemon.mMoveSet[cursorRow]->mName.GetString() + "!#@" +
                    "And... # #+END",
                    mWorld
                );
                
                activePlayerPokemon.mMoveSet[cursorRow]     = nullptr;
                activePlayerPokemon.mMovesetIndexForNewMove = cursorRow;
            }
            else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
            {
                TransitionToAbandonLearningNewMoveQuestion();
            }
        }
        else
        {
            CreateEncounterFightMenuTextbox(activePlayerPokemon.mMoveSet, 0, mWorld, MOVES_TEXTBOX_POSITION);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void SelectWhichMoveToReplaceFlowState::TransitionToAbandonLearningNewMoveQuestion()
{
    // Destroy Move Selection Textbox
    DestroyActiveTextbox(mWorld);
    
    // Destroy Chatbox
    DestroyActiveTextbox(mWorld);
    
    CompleteAndTransitionTo<AbandonLearningNewMoveFlowState>();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


