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
#include "../components/CursorComponent.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

FightMenuEncounterFlowState::FightMenuEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    PokemonMoveSet moveset;
    moveset[0] = { StringId("THUNDERSHOCK"), StringId("ELECTRIC"), 30, 30};
    moveset[1] = { StringId("GROWL"), StringId("NORMAL"), 40, 40 };
    moveset[2] = { StringId("-"), StringId("NORMAL"), 0, 0 };
    moveset[3] = { StringId("-"), StringId("NORMAL"), 0, 0 };
    CreateEncounterFightMenuTextbox(moveset, encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu, mWorld);
    encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId = CreateEncounterFightMenuMoveInfoTextbox(moveset[encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu], mWorld);
}

void FightMenuEncounterFlowState::VUpdate(const float)
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (IsActionTypeKeyTapped(VirtualActionType::A, inputStateComponent))
    {

    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B, inputStateComponent))
    {
        // Destroy fight menu textbox
        DestroyActiveTextbox(mWorld);

        // Destroy move info textbox
        DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId, mWorld);

        CompleteAndTransitionTo<MainMenuEncounterFlowState>();
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::UP, inputStateComponent) || IsActionTypeKeyTapped(VirtualActionType::DOWN, inputStateComponent))
    {        
        // Destroy move info textbox
        DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId, mWorld);

        const auto& cursorComponent = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
        
        // TEMP
        PokemonMoveSet moveset;
        moveset[0] = { StringId("THUNDERSHOCK"), StringId("ELECTRIC"), 30, 30 };
        moveset[1] = { StringId("GROWL"), StringId("NORMAL"), 40, 40 };
        moveset[2] = { StringId("-"), StringId("NORMAL"), 0, 0 };
        moveset[3] = { StringId("-"), StringId("NORMAL"), 0, 0 };

        encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId = CreateEncounterFightMenuMoveInfoTextbox(moveset[cursorComponent.mCursorRow], mWorld);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

