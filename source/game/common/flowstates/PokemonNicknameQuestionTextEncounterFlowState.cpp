//
//  PokemonNicknameQuestionTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonNicknameQuestionTextEncounterFlowState.h"
#include "DeterminePokemonPlacementFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../../common/components/CursorComponent.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PokemonNicknameQuestionTextEncounterFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.065f, -0.4f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokemonNicknameQuestionTextEncounterFlowState::PokemonNicknameQuestionTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& encounterStsateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto mainChatboxEntityId       = CreateChatbox(mWorld);
    
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        "Do you want to#give a nickname#to " +
        encounterStsateComponent.mOpponentPokemonRoster.at(0)->mName.GetString() +
        "?+FREEZE",
        mWorld
    );
}

void PokemonNicknameQuestionTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent   = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    
    // Nickname question chatbox active
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
        {
            CreateYesNoTextbox(mWorld, YES_NO_TEXTBOX_POSITION);
        }
    }
    // Yes/No textbox active
    else
    {
        const auto& cursorComponent   = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
        
        if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
        {
            if (cursorComponent.mCursorRow == 0)
            {
                //TODO: Nickname Flow
            }
            else
            {
                DestroyTextboxesAndTransition();
            }
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            DestroyTextboxesAndTransition();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PokemonNicknameQuestionTextEncounterFlowState::DestroyTextboxesAndTransition()
{        
    // Destroy Yes/No textbox
    DestroyActiveTextbox(mWorld);
    
    // Destroy Nickname Chatbox
    DestroyActiveTextbox(mWorld);
    
    CompleteAndTransitionTo<DeterminePokemonPlacementFlowState>();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
