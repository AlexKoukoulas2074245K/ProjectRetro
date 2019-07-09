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
    moveset[0] = { StringId("THUNDERSHOCK"), 30 };
    moveset[1] = { StringId("GROWL"), 40 };
    moveset[2] = { StringId("-"), 0 };
    moveset[3] = { StringId("-"), 0 };
    CreateEncounterFightMenuTextbox(moveset, encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu, mWorld);
}

void FightMenuEncounterFlowState::VUpdate(const float)
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();

    if (IsActionTypeKeyTapped(VirtualActionType::A, inputStateComponent))
    {

    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B, inputStateComponent))
    {
        DestroyActiveTextbox(mWorld);
        CompleteAndTransitionTo<MainMenuEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

