//
//  StatusChangeTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "StatusChangeTextEncounterFlowState.h"
#include "TurnOverEncounterFlowState.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

StatusChangeTextEncounterFlowState::StatusChangeTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if 
    (
        encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon == PokemonStatus::NORMAL &&
        encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon == PokemonStatus::NORMAL
    )
    {
        CompleteAndTransitionTo<TurnOverEncounterFlowState>();
        return;
    }

    const auto mainChatboxEntityId = CreateChatbox(world);

    if (encounterStateComponent.mIsOpponentsTurn)
    {
        //// AAAA#was poisoned!
        // once textbox is dead change status

        //// Enemy AAAAA's#paralyzed! It may#not attack!+END
    }         
}

void StatusChangeTextEncounterFlowState::VUpdate(const float)
{  
    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
