//
//  FirstTurnOverEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "FirstTurnOverEncounterFlowState.h"
#include "MainMenuEncounterFlowState.h"
#include "PokemonDeathMovementEncounterFlowState.h"
#include "DamageCalculationEncounterFlowState.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/PokemonMoveUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

FirstTurnOverEncounterFlowState::FirstTurnOverEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
}

void FirstTurnOverEncounterFlowState::VUpdate(const float)
{
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    encounterStateComponent.mIsOpponentsTurn = !encounterStateComponent.mIsOpponentsTurn;
    
    
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster.front();
    auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster.front();
    
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
            const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster.front();
            
            // Ultra smart AI
            auto& opponentSelectedMove = *activeOpponentPokemon.mMoveSet[math::RandomInt(0, FindFirstUnusedMoveIndex(activeOpponentPokemon.mMoveSet) - 1)];
            
            encounterStateComponent.mLastMoveSelected = opponentSelectedMove.mName;
        }
        else
        {
            //TODO: move PP check
            auto& playerStateComponent      = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
            const auto& activePlayerPokemon = *playerStateComponent.mPlayerPokemonRoster.front();
            
            auto& playerSelectedMove = *activePlayerPokemon.mMoveSet[encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu];
            
            playerSelectedMove.mPowerPointsLeft--;
            encounterStateComponent.mLastMoveSelected = playerSelectedMove.mName;
        }
        
        CompleteAndTransitionTo<DamageCalculationEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

