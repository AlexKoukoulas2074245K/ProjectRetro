//
//  AbandonLearningNewMoveFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 04/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "AbandonLearningNewMoveFlowState.h"
#include "FullMovesetIntroTextFlowState.h"
#include "NextOpponentPokemonCheckEncounterFlowState.h"
#include "TrainerBattleWonEncounterFlowState.h"
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

const glm::vec3 AbandonLearningNewMoveFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.058000f, -0.1f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

AbandonLearningNewMoveFlowState::AbandonLearningNewMoveFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activePlayerPokemon  = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        "Abandon learning#" + activePlayerPokemon.mMoveToBeLearned.GetString() + "?+FREEZE",
        mWorld
    );
}

void AbandonLearningNewMoveFlowState::VUpdate(const float)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& playerStateComponent          = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    // Abandoned learning move text dismissed
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        playerStateComponent.mLeveledUpPokemonRosterIndex = -1;
        if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
        {
            if (GetFirstNonFaintedPokemonIndex(encounterStateComponent.mOpponentPokemonRoster) != encounterStateComponent.mOpponentPokemonRoster.size())
            {
                CompleteAndTransitionTo<NextOpponentPokemonCheckEncounterFlowState>();
            }
            else
            {
                CompleteAndTransitionTo<TrainerBattleWonEncounterFlowState>();
            }
        }
        else
        {
            //TODO: Continue with overworld
        }
    }
    else if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
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
                    AbandonLearningNewMove();
                }
                // No selected
                else
                {
                    TransitionToFullMovesetState();
                }
            }
            else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
            {
                TransitionToFullMovesetState();
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

void AbandonLearningNewMoveFlowState::TransitionToFullMovesetState()
{
    // Destroy Yes/No textbox
    DestroyActiveTextbox(mWorld);
    
    // Destroy Chatbox
    DestroyActiveTextbox(mWorld);

    CompleteAndTransitionTo<FullMovesetIntroTextFlowState>();
}

void AbandonLearningNewMoveFlowState::AbandonLearningNewMove()
{
    // Destroy Yes/No textbox
    DestroyActiveTextbox(mWorld);
    
    // Destroy Chatbox
    DestroyActiveTextbox(mWorld);
    
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activePlayerPokemon  = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        activePlayerPokemon.mName.GetString() + "#did not learn#" + activePlayerPokemon.mMoveToBeLearned.GetString() + "!#+END",
        mWorld
    );
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
