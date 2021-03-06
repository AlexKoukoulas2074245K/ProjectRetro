//
//  TrainerBattleWonDefeatedTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TrainerBattleWonDefeatedTextEncounterFlowState.h"
#include "TrainerBattleWonPayoutEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TrainerBattleWonDefeatedTextEncounterFlowState::TrainerBattleWonDefeatedTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    const auto mainChatboxEntityId = CreateChatbox(world);
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        encounterStateComponent.mOpponentTrainerDefeatedText.GetString() + "#+END",
        mWorld
    );
}

void TrainerBattleWonDefeatedTextEncounterFlowState::VUpdate(const float)
{   
    const auto& guiStateSingletonComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateSingletonComponent.mActiveTextboxesStack.size() == 1)
    {        
        CompleteAndTransitionTo<TrainerBattleWonPayoutEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
