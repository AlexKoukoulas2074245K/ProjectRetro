//
//  UseNextPokemonQuestionEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "UseNextPokemonQuestionEncounterFlowState.h"
#include "PokemonSelectionViewFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string UseNextPokemonQuestionEncounterFlowState::RUN_AWAY_SAFELY_SFX_NAME = "encounter/run";

const glm::vec3 UseNextPokemonQuestionEncounterFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.412899911f, -0.275700212, -0.1f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

UseNextPokemonQuestionEncounterFlowState::UseNextPokemonQuestionEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto mainChatboxEntityId = CreateChatbox(world);    
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        "Use next POK^MON?+FREEZE",
        mWorld
    );
}

void UseNextPokemonQuestionEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // Got away safely text has just been discarded
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
        encounterStateComponent.mEncounterJustFinished = true;
    }
    // Use next pokemon text has finished rendering
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
                    TransitionToPokemonSelectionView();
                }
                // No selected
                else
                {
                    RenderGotAwaySafelyText();
                }
            }
            else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
            {
                RenderGotAwaySafelyText();
            }            
        }
        // Yes-No textbox has not been created yet
        else
        {
            CreateYesNoTextbox(mWorld, glm::vec3(YES_NO_TEXTBOX_POSITION.x, YES_NO_TEXTBOX_POSITION.y, YES_NO_TEXTBOX_POSITION.z));
        }        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void UseNextPokemonQuestionEncounterFlowState::TransitionToPokemonSelectionView()
{
    // Destroy Yes/No textbox
    DestroyActiveTextbox(mWorld);

    // Destroy use next pokemon chatbox
    DestroyActiveTextbox(mWorld);

    auto& pokemonSelectionViewStateComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    pokemonSelectionViewStateComponent.mCreationSourceType = PokemonSelectionViewCreationSourceType::ENCOUNTER_AFTER_POKEMON_FAINTED;

    CompleteAndTransitionTo<PokemonSelectionViewFlowState>();
}

void UseNextPokemonQuestionEncounterFlowState::RenderGotAwaySafelyText()
{
    // Destroy Yes/No textbox
    DestroyActiveTextbox(mWorld);

    // Destroy use next pokemon chatbox
    DestroyActiveTextbox(mWorld);

    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox(mainChatboxEntityId, "Got away safely!# #+END", mWorld);

    SoundService::GetInstance().PlaySfx(RUN_AWAY_SAFELY_SFX_NAME);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
