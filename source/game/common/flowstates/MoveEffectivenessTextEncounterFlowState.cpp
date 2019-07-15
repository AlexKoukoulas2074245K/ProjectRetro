//
//  MoveEffectivenessTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 14/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MoveEffectivenessTextEncounterFlowState.h"
#include "FirstTurnOverEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


MoveEffectivenessTextEncounterFlowState::MoveEffectivenessTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // This is in all cases 2, except for when a critical hit is achieved
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        DestroyActiveTextbox(mWorld);
    }    
}

void MoveEffectivenessTextEncounterFlowState::VUpdate(const float)
{
    CompleteAndTransitionTo<FirstTurnOverEncounterFlowState>();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

