//
//  MainMenuEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 08/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MainMenuEncounterFlowState.h"
#include "FightMenuEncounterFlowState.h"
#include "ItemMenuFlowState.h"
#include "PokemonSelectionViewFlowState.h"
#include "PreDamageCalculationChecksEncounterFlowState.h"
#include "RunAttemptEncounterFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../common/utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MainMenuEncounterFlowState::MainMenuEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    CreateEncounterMainMenuTextbox(encounterStateComponent.mLastEncounterMainMenuActionSelected, world);
}

void MainMenuEncounterFlowState::VUpdate(const float)
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {        
        auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
        const auto& cursorComponent   = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
        
        const auto cursorCol = cursorComponent.mCursorCol;
        const auto cursorRow = cursorComponent.mCursorRow;
        
        DestroyActiveTextbox(mWorld);

        if (cursorCol == 0 && cursorRow == 0)
        {
            // Destroy encounter main menu
            encounterStateComponent.mLastEncounterMainMenuActionSelected = MainMenuActionType::FIGHT;
            if (ShouldSkipMoveSelection())
            {
                CompleteAndTransitionTo<PreDamageCalculationChecksEncounterFlowState>();
            }
            else
            {
                CompleteAndTransitionTo<FightMenuEncounterFlowState>();
            }
            
        }
        else if (cursorCol == 1 && cursorRow == 0)
        {
            auto& pokemonSelectionViewState = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
            pokemonSelectionViewState.mCreationSourceType = PokemonSelectionViewCreationSourceType::ENCOUNTER_FROM_MAIN_MENU;
            encounterStateComponent.mLastEncounterMainMenuActionSelected = MainMenuActionType::POKEMON;            
            CompleteAndTransitionTo<PokemonSelectionViewFlowState>();
        }
        else if (cursorCol == 0 && cursorRow == 1)
        {
            encounterStateComponent.mLastEncounterMainMenuActionSelected = MainMenuActionType::ITEM;
            CompleteAndTransitionTo<ItemMenuFlowState>();
        }
        else if (cursorCol == 1 && cursorRow == 1)
        {
            encounterStateComponent.mLastEncounterMainMenuActionSelected = MainMenuActionType::RUN;            
            CompleteAndTransitionTo<RunAttemptEncounterFlowState>();
        }        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

bool MainMenuEncounterFlowState::ShouldSkipMoveSelection() const
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    auto& activePlayerPokemon = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];

    if
    (
        encounterStateComponent.mIsOpponentsTurn == false &&
        (
            activePlayerPokemon.mNumberOfRoundsUntilSleepEnds > 0 ||
            activePlayerPokemon.mStatus == PokemonStatus::ASLEEP
        )
    )
    {
        return true;
    }
    else if
    (
        encounterStateComponent.mIsOpponentsTurn == false &&
        activePlayerPokemon.mBideCounter > -1
    )
    {
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
