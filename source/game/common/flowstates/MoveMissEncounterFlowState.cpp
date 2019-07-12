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
#include "MainMenuEncounterFlowState.h"
#include "DamageCalculationEncounterFlowState.h"
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
    const auto mainChatboxEntityId = CreateChatbox(world);
    
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();    
    
    const auto& attackingPokemon = encounterStateComponent.mIsOpponentsTurn ?
        encounterStateComponent.mOpponentPokemonRoster.front() :
        playerStateComponent.mPlayerPokemonRoster.front();

    QueueDialogForTextbox
    (
        mainChatboxEntityId,
        attackingPokemon->mName.GetString() + "'s#attack missed!+END",
        mWorld
    );
}

void MoveMissEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        encounterStateComponent.mIsOpponentsTurn = !encounterStateComponent.mIsOpponentsTurn;

        // Both player turns are over
        if (++encounterStateComponent.mTurnsCompleted == 2)
        {
            CompleteAndTransitionTo<MainMenuEncounterFlowState>();
        }
        else
        {
            CompleteAndTransitionTo<DamageCalculationEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
