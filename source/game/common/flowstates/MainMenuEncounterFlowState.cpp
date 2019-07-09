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
#include "../components/CursorComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../common/utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MainMenuEncounterFlowState::MainMenuEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    CreateEncounterMainMenuTextbox(encounterStateComponent.mLastEncounterMainMenuActionSelected, world);
}

void MainMenuEncounterFlowState::VUpdate(const float)
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    if (IsActionTypeKeyTapped(VirtualActionType::A, inputStateComponent))
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
            CompleteAndTransitionTo<FightMenuEncounterFlowState>();
        }
        else if (cursorCol == 1 && cursorRow == 0)
        {
            encounterStateComponent.mLastEncounterMainMenuActionSelected = MainMenuActionType::POKEMON;
            //TODO: create actual one
            CompleteAndTransitionTo<FightMenuEncounterFlowState>();
        }
        else if (cursorCol == 0 && cursorRow == 1)
        {
            encounterStateComponent.mLastEncounterMainMenuActionSelected = MainMenuActionType::ITEM;
            //TODO: create actual one
            CompleteAndTransitionTo<FightMenuEncounterFlowState>();
        }
        else if (cursorCol == 1 && cursorRow == 1)
        {
            encounterStateComponent.mLastEncounterMainMenuActionSelected = MainMenuActionType::RUN;
            //TODO: create actual one
            CompleteAndTransitionTo<FightMenuEncounterFlowState>();
        }        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

