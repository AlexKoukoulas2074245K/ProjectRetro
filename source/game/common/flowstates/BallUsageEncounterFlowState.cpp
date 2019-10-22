//
//  BallUsageEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 08/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BallAnimationEncounterFlowState.h"
#include "BallUsageEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

BallUsageEncounterFlowState::BallUsageEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
}

void BallUsageEncounterFlowState::VUpdate(const float)
{          
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent       = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    if (encounterStateComponent.mIsPikachuCaptureFlowActive)
    {
        if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
        {
            encounterStateComponent.mWasPokemonCaught     = true;
            encounterStateComponent.mBallThrownShakeCount = 3;
            
            CompleteAndTransitionTo<BallAnimationEncounterFlowState>();
        }
        
        return;
    }
    
    const auto& itemBagEntry = playerStateComponent.mPlayerBag.at
    (
        playerStateComponent.mPreviousItemMenuItemOffset +
        playerStateComponent.mPreviousItemMenuCursorRow
    );

    if (encounterStateComponent.mActiveEncounterType == EncounterType::TRAINER)
    {
        encounterStateComponent.mWasPokemonCaught = false;
        encounterStateComponent.mBallThrownShakeCount = -1;

        CompleteAndTransitionTo<BallAnimationEncounterFlowState>();
    }
    else if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        encounterStateComponent.mWasPokemonCaught     = false;
        encounterStateComponent.mBallThrownShakeCount = -1;

        if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
        {
            const auto catchResult = CalculateCatchResult(itemBagEntry.mItemName, *encounterStateComponent.mOpponentPokemonRoster.at(0));
            encounterStateComponent.mWasPokemonCaught     = catchResult.mWasCaught;
            encounterStateComponent.mBallThrownShakeCount = catchResult.mNumberOfBallShakes;

            CompleteAndTransitionTo<BallAnimationEncounterFlowState>();
        }        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
