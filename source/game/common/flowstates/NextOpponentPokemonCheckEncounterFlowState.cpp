//
//  NextOpponentPokemonCheckEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "NextOpponentPokemonCheckEncounterFlowState.h"
#include "OpponentTrainerPokemonSummonTextEncounterFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../utils/PokemonUtils.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "PokemonSelectionViewFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 NextOpponentPokemonCheckEncounterFlowState::OPPONENT_ROSTER_DISPLAY_POSITION = glm::vec3(-0.32f, 0.7f, 0.4f);
const glm::vec3 NextOpponentPokemonCheckEncounterFlowState::OPPONENT_ROSTER_DISPLAY_SCALE    = glm::vec3(1.04f, 1.04f, 1.0f);
const glm::vec3 NextOpponentPokemonCheckEncounterFlowState::OPPONENT_INFO_TEXTBOX_POSITION   = glm::vec3(-0.2085f, 0.796f, 0.35f);

glm::vec3 NextOpponentPokemonCheckEncounterFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(-0.481498629f, -0.058000f, -0.1f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

NextOpponentPokemonCheckEncounterFlowState::NextOpponentPokemonCheckEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    encounterStateComponent.mActiveOpponentPokemonRosterIndex = GetFirstNonFaintedPokemonIndex(encounterStateComponent.mOpponentPokemonRoster);

    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& nextOpponentPokemon     = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        DestroyActiveTextbox(mWorld);
    }

    CreateOpponentRosterDisplay();

    const auto mainChatboxEntityId = CreateChatbox(world);
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        encounterStateComponent.mOpponentTrainerName.GetString() + " is#about to use#" + 
        nextOpponentPokemon.mName.GetString() + "!#@Will " + 
        playerStateComponent.mTrainerName.GetString() + "#change POK^MON?+FREEZE",
        mWorld
    );
}

void NextOpponentPokemonCheckEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        const auto activeTextboxEntityId   = GetActiveTextboxEntityId(mWorld);
        const auto& activeTextboxComponent = mWorld.GetComponent<TextboxComponent>(activeTextboxEntityId);

        // Yes-No textbox has already been created
        if (activeTextboxComponent.mTextboxType == TextboxType::CURSORED_TEXTBOX)
        {
            const auto& cursorComponent = mWorld.GetComponent<CursorComponent>(activeTextboxEntityId);
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
                    TransitionToNextOpponentPokemonState();
                }
            }
            else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
            {
                TransitionToNextOpponentPokemonState();
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

void NextOpponentPokemonCheckEncounterFlowState::CreateOpponentRosterDisplay() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId = LoadAndCreateOpponentRosterDisplay
    (
        OPPONENT_ROSTER_DISPLAY_POSITION,
        OPPONENT_ROSTER_DISPLAY_SCALE,
        mWorld
    );

    // Pokemon party pokeball rendering
    encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::BARE_TEXTBOX,
        20,
        2,
        OPPONENT_INFO_TEXTBOX_POSITION.x,
        OPPONENT_INFO_TEXTBOX_POSITION.y,
        OPPONENT_INFO_TEXTBOX_POSITION.z,
        mWorld
    );

    const auto& opponentPokemonRoster = encounterStateComponent.mOpponentPokemonRoster;
    for (auto i = 0U; i < opponentPokemonRoster.size(); ++i)
    {
        const auto& pokemon = *opponentPokemonRoster[i];

        std::string statusString = "~"; // normal status

        if (pokemon.mHp <= 0)
        {
            statusString = "+";
        }
        else if (pokemon.mStatus != PokemonStatus::NORMAL)
        {
            statusString = "`";
        }

        // select normal pokeball, status or faint
        WriteTextAtTextboxCoords
        (
            encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId,
            statusString,
            10 - i,
            1,
            mWorld
        );
    }

    for (auto i = opponentPokemonRoster.size(); i < 6U; ++i)
    {
        // select normal pokeball, status or faint
        WriteTextAtTextboxCoords
        (
            encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId,
            "\"",
            10 - i,
            1,
            mWorld
        );
    }
}

void NextOpponentPokemonCheckEncounterFlowState::TransitionToPokemonSelectionView()
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& pokemonSelectionViewStateComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    // Destroy Yes/No textbox
    DestroyActiveTextbox(mWorld);

    // Destroy Chatbox
    DestroyActiveTextbox(mWorld);
    
    pokemonSelectionViewStateComponent.mCreationSourceType = PokemonSelectionViewCreationSourceType::ENCOUNTER_AFTER_POKEMON_FAINTED;    
    
    DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, mWorld);
    mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId);

    encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId = ecs::NULL_ENTITY_ID;
    encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId = ecs::NULL_ENTITY_ID;
    encounterStateComponent.mOpponentPendingSummoning = true;
    encounterStateComponent.mPlayerDecidedToChangePokemonBeforeNewOpponentPokemonIsSummoned = true;

    CompleteAndTransitionTo<PokemonSelectionViewFlowState>();
}

void NextOpponentPokemonCheckEncounterFlowState::TransitionToNextOpponentPokemonState()
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    // Destroy Yes-No textbox
    DestroyActiveTextbox(mWorld);

    // Destroy Chatbox
    DestroyActiveTextbox(mWorld);

    DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, mWorld);
    mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId);

    encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId                = ecs::NULL_ENTITY_ID;
    encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId                     = ecs::NULL_ENTITY_ID;
    encounterStateComponent.mOpponentPendingSummoning                                       = true;
    encounterStateComponent.mPlayerDecidedToChangePokemonBeforeNewOpponentPokemonIsSummoned = false;

    CompleteAndTransitionTo<OpponentTrainerPokemonSummonTextEncounterFlowState>();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

