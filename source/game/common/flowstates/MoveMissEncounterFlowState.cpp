//
//  MoveMissEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MoveMissEncounterFlowState.h"
#include "PoisonTickCheckEncounterFlowState.h"
#include "../utils/PokemonMoveUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MoveMissEncounterFlowState::MoveMissEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    DestroyActiveTextbox(mWorld);
    
    const auto mainChatboxEntityId = CreateChatbox(world);
    
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();    
    
    const auto& attackingPokemon = encounterStateComponent.mIsOpponentsTurn ?
        *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex]:
        *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];

    const auto& lastMoveUsedStats = GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld);
    
    if (lastMoveUsedStats.mPower == 0)
    {
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            "But, it failed!# #+END",
            mWorld
        );
    }
    else
    {
        if (encounterStateComponent.mIsOpponentsTurn)
        {
            QueueDialogForChatbox
            (
                mainChatboxEntityId,
                "Enemy " + attackingPokemon.mName.GetString() + "'s#attack missed!#+END",
                mWorld
            );
        }
        else
        {
            QueueDialogForChatbox
            (
                mainChatboxEntityId,
                attackingPokemon.mName.GetString() + "'s#attack missed!#+END",
                mWorld
            );
        }
    }
}

void MoveMissEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        CompleteAndTransitionTo<PoisonTickCheckEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
