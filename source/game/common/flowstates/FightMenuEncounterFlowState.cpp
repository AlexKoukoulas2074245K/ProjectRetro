//
//  FightMenuEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 09/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "FightMenuEncounterFlowState.h"
#include "MainMenuEncounterFlowState.h"
#include "NoPPLeftTextEncounterFlowState.h"
#include "RoundStructureCalculationEncounterFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

FightMenuEncounterFlowState::FightMenuEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& playerPokemonMoveset = playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mMoveSet;
    
    if (encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId == ecs::NULL_ENTITY_ID)
    {
        CreateEncounterFightMenuTextbox(playerPokemonMoveset, encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu, mWorld);
        encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId = CreateEncounterFightMenuMoveInfoTextbox(*playerPokemonMoveset[encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu], mWorld);
    }    
}

void FightMenuEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();    
    const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();    
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();    
    const auto& activePlayerPokemon  = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        const auto& cursorComponent = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));

        if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
        {
            encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu = cursorComponent.mCursorRow;
            auto& playerSelectedMove = *activePlayerPokemon.mMoveSet[encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu];

            if (playerSelectedMove.mPowerPointsLeft == 0)
            {
                CompleteAndTransitionTo<NoPPLeftTextEncounterFlowState>();
                return;
            }

            // Destroy fight menu textbox
            DestroyActiveTextbox(mWorld);

            // Destroy move info textbox
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId, mWorld);
            encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId = ecs::NULL_ENTITY_ID;

            encounterStateComponent.mNumberOfEscapeAttempts = 0;

            CompleteAndTransitionTo<RoundStructureCalculationEncounterFlowState>();
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            // Destroy fight menu textbox
            DestroyActiveTextbox(mWorld);

            // Destroy move info textbox
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId, mWorld);
            encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId = ecs::NULL_ENTITY_ID;

            CompleteAndTransitionTo<MainMenuEncounterFlowState>();
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent) || IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent))
        {
            // Destroy move info textbox
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId, mWorld);            
            const auto& playerPokemonMoveset = playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mMoveSet;
            encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId = CreateEncounterFightMenuMoveInfoTextbox(*playerPokemonMoveset[cursorComponent.mCursorRow], mWorld);
        }
    }   
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

