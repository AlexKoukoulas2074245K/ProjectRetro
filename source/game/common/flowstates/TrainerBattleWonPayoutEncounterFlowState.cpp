//
//  TrainerBattleWonPayoutEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "EvolutionTextFlowState.h"
#include "TrainerBattleWonPayoutEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../utils/TrainerUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TrainerBattleWonPayoutEncounterFlowState::TrainerBattleWonPayoutEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto payout = CalculateTrainerBattleWonPayout(mWorld);

    auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    playerStateComponent.mPokeDollarCredits += payout;

    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox
    (
        mainChatboxEntityId, 
        playerStateComponent.mTrainerName.GetString() + " got $" + std::to_string(payout) + "#for winning!#+END",
        mWorld
    );
}

void TrainerBattleWonPayoutEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {        
        CompleteAndTransitionTo<EvolutionTextFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
