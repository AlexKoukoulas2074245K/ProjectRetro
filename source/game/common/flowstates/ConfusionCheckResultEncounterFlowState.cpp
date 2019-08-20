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
#include "../utils/PokemonMoveUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ConfusionCheckResultEncounterFlowState::ConfusionCheckResultEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto shouldHurtItself = math::RandomSign() == 1;    
    
    // Destroy confusion check textbox
    DestroyActiveTextbox(mWorld);

    if (shouldHurtItself)
    {
        encounterStateComponent.mLastMoveSelected = CONFUSION_HURT_ITSELF_MOVE_NAME;
        const auto mainChatboxEntityId = CreateChatbox(mWorld);
        QueueDialogForChatbox(mainChatboxEntityId, "It hurt itself in#its confusion!#+END", mWorld);
    }    
}

void ConfusionCheckResultEncounterFlowState::VUpdate(const float)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (encounterStateComponent.mLastMoveSelected != CONFUSION_HURT_ITSELF_MOVE_NAME)
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
