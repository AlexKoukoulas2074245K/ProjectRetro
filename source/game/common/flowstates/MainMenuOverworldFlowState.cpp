//
//  MainMenuOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 08/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ItemMenuFlowState.h"
#include "MainMenuOverworldFlowState.h"
#include "PokemonSelectionViewFlowState.h"
#include "SavePromptDialogOverworldFlowState.h"
#include "TrainerCardOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokedexStateSingletonComponent.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../input/utils/InputUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../overworld/components/OverworldFlowStateSingletonComponent.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string MainMenuOverworldFlowState::TEXTBOX_CLICK_SFX_NAME = "general/textbox_click";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MainMenuOverworldFlowState::MainMenuOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{    
    const auto& playerStateComponent  = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        CreateOverworldMainMenuTextbox(world, HasMilestone(milestones::RECEIVED_POKEDEX, mWorld), playerStateComponent.mPreviousMainMenuCursorRow);
    }
}

void MainMenuOverworldFlowState::VUpdate(const float)
{
    const auto& inputStateComponent          = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& pokemonSelectionViewStateComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        const auto mainMenuItem = GetCursorMainMenuItemFirstFourLetters();
        if (StringStartsWith(mainMenuItem, "POK^DE"))
        {
            
        }
        else if (StringStartsWith(mainMenuItem, "POK^MO"))
        {
            
            const auto& cursorComponent = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
            auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
            playerStateComponent.mPreviousMainMenuCursorRow = cursorComponent.mCursorRow;

            if (playerStateComponent.mPlayerPokemonRoster.size() > 0)
            {
                // Destory main menu textbox
                DestroyActiveTextbox(mWorld);
                        
                pokemonSelectionViewStateComponent.mCreationSourceType = PokemonSelectionViewCreationSourceType::OVERWORLD;
                CompleteAndTransitionTo<PokemonSelectionViewFlowState>();
            }
        }
        else if (StringStartsWith(mainMenuItem, "ITEM"))
        {
            CompleteAndTransitionTo<ItemMenuFlowState>();
        }
        else if (StringStartsWith(mainMenuItem, "SAVE"))
        {
            CompleteAndTransitionTo<SavePromptDialogOverworldFlowState>();
        }
        else if (StringStartsWith(mainMenuItem, "OPTIO"))
        {
            
        }
        else if (StringStartsWith(mainMenuItem, "EXIT"))
        {
            ExitOverworldMainMenu();
        }        
        else
        {
            CompleteAndTransitionTo<TrainerCardOverworldFlowState>();
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        ExitOverworldMainMenu();
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::START_BUTTON, inputStateComponent) && inputStateComponent.mHasBeenConsumed == false)
    {
        ExitOverworldMainMenu();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void MainMenuOverworldFlowState::ExitOverworldMainMenu()
{
    SoundService::GetInstance().PlaySfx(TEXTBOX_CLICK_SFX_NAME);
    
    const auto& cursorComponent = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
    auto& playerStateComponent  = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    playerStateComponent.mPreviousMainMenuCursorRow = cursorComponent.mCursorRow;

    // Destroy main menu
    DestroyActiveTextbox(mWorld);
    
    CompleteOverworldFlow();
}

std::string MainMenuOverworldFlowState::GetCursorMainMenuItemFirstFourLetters() const
{
    const auto activeTextboxEntityId   = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent        = mWorld.GetComponent<CursorComponent>(activeTextboxEntityId);
    const auto& activeTextboxComponent = mWorld.GetComponent<TextboxComponent>(activeTextboxEntityId);
    
    std::string menuItemFirstSixLetters;
    menuItemFirstSixLetters.push_back(activeTextboxComponent.mTextContent[cursorComponent.mCursorRow * 2 + 2][2].mCharacter);
    menuItemFirstSixLetters.push_back(activeTextboxComponent.mTextContent[cursorComponent.mCursorRow * 2 + 2][3].mCharacter);
    menuItemFirstSixLetters.push_back(activeTextboxComponent.mTextContent[cursorComponent.mCursorRow * 2 + 2][4].mCharacter);
    menuItemFirstSixLetters.push_back(activeTextboxComponent.mTextContent[cursorComponent.mCursorRow * 2 + 2][5].mCharacter);
    menuItemFirstSixLetters.push_back(activeTextboxComponent.mTextContent[cursorComponent.mCursorRow * 2 + 2][6].mCharacter);
    menuItemFirstSixLetters.push_back(activeTextboxComponent.mTextContent[cursorComponent.mCursorRow * 2 + 2][7].mCharacter);
    
    return menuItemFirstSixLetters;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

