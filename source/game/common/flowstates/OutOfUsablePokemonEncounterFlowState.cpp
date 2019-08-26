//
//  OutOfUsablePokemonEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 04/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OutOfUsablePokemonEncounterFlowState.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../overworld/components/TransitionAnimationStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OutOfUsablePokemonEncounterFlowState::OutOfUsablePokemonEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& playerStateComponent         = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& transitionAnimationComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    
    playerStateComponent.mPokeDollarCredits /= 2;
    transitionAnimationComponent.mBlackAndWhiteModeEnabled = true;
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        playerStateComponent.mPlayerTrainerName.GetString() + " is out of#useable POK^MON!#@" +
        playerStateComponent.mPlayerTrainerName.GetString() + " blacked#out!#+END",
        mWorld
    );
}

void OutOfUsablePokemonEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent      = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent      = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        encounterStateComponent.mEncounterJustFinished = true;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

