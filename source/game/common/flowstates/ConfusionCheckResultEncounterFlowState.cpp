//
//  ConfusionCheckResultEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 16/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ConfusionCheckResultEncounterFlowState.h"
#include "ConfusionHurtItselfAnimationEncounterFlowState.h"
#include "DamageCalculationEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/MathUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ConfusionCheckResultEncounterFlowState::ConfusionCheckResultEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    encounterStateComponent.mDidConfusedPokemonHurtItself = math::RandomSign() == 1;    
    
    // Destroy confusion check textbox
    DestroyActiveTextbox(mWorld);

    if (encounterStateComponent.mDidConfusedPokemonHurtItself)
    {
        const auto mainChatboxEntityId = CreateChatbox(mWorld);
        QueueDialogForChatbox(mainChatboxEntityId, "It hurt itself in#its confusion!#+END", mWorld);
    }    
}

void ConfusionCheckResultEncounterFlowState::VUpdate(const float)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (encounterStateComponent.mDidConfusedPokemonHurtItself == false)
    {
        CompleteAndTransitionTo<DamageCalculationEncounterFlowState>();
    }
    else
    {
        if (guiStateComponent.mActiveTextboxesStack.size() == 1)
        {
            const auto mainChatboxEntityId = CreateChatbox(mWorld);
            WriteTextAtTextboxCoords(mainChatboxEntityId, "It hurt itself in", 1, 2, mWorld);
            WriteTextAtTextboxCoords(mainChatboxEntityId, "its confusion!", 1, 4, mWorld);

            CompleteAndTransitionTo<ConfusionHurtItselfAnimationEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
