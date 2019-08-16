//
//  FirstTurnOverEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TurnOverEncounterFlowState.h"
#include "MainMenuEncounterFlowState.h"
#include "PokemonDeathMovementEncounterFlowState.h"
#include "PreDamageCalculationChecksEncounterFlowState.h"
#include "../utils/PokemonUtils.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/PokemonMoveUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TurnOverEncounterFlowState::TurnOverEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
}

void TurnOverEncounterFlowState::VUpdate(const float)
{
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    encounterStateComponent.mDidConfusedPokemonHurtItself = false;
    encounterStateComponent.mIsOpponentsTurn = !encounterStateComponent.mIsOpponentsTurn;    
    
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    
    if (activePlayerPokemon.mHp <= 0)
    {
        activePlayerPokemon.mHp = 0;
        CompleteAndTransitionTo<PokemonDeathMovementEncounterFlowState>();
        return;
    }
    else if (activeOpponentPokemon.mHp <= 0)
    {
        activeOpponentPokemon.mHp = 0;
        CompleteAndTransitionTo<PokemonDeathMovementEncounterFlowState>();
        return;
    }
    
    // Both player turns are over
    if (++encounterStateComponent.mTurnsCompleted == 2)
    {
        CompleteAndTransitionTo<MainMenuEncounterFlowState>();
    }
    else
    {
        if (encounterStateComponent.mIsOpponentsTurn)
        {            
            // Ultra smart AI
            auto& opponentSelectedMove = *activeOpponentPokemon.mMoveSet[math::RandomInt(0, FindFirstUnusedMoveIndex(activeOpponentPokemon.mMoveSet) - 1)];
            
            encounterStateComponent.mLastMoveSelected = opponentSelectedMove.mName;
        }
        else
        {
            auto& playerSelectedMove = *activePlayerPokemon.mMoveSet[encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu];
            
            playerSelectedMove.mPowerPointsLeft--;
            encounterStateComponent.mLastMoveSelected = playerSelectedMove.mName;
        }
        

        CompleteAndTransitionTo<PreDamageCalculationChecksEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

