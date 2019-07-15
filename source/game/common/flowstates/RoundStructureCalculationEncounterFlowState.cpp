//
//  RoundStructureCalculationEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "RoundStructureCalculationEncounterFlowState.h"
#include "DamageCalculationEncounterFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../common/utils/MathUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

RoundStructureCalculationEncounterFlowState::RoundStructureCalculationEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    const auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster.front();
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster.front();

    encounterStateComponent.mLastMoveMiss    = false;
    encounterStateComponent.mLastMoveCrit    = false;
    encounterStateComponent.mIsOpponentsTurn = false;
    encounterStateComponent.mTurnsCompleted  = 0;

    auto& playerSelectedMove   = *activePlayerPokemon.mMoveSet[encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu];
    
    // Ultra smart AI
    auto& opponentSelectedMove = *activeOpponentPokemon.mMoveSet[math::RandomInt(0, FindFirstUnusedMoveIndex(activeOpponentPokemon.mMoveSet) - 1)];

    if 
    (
        ShouldOpponentGoFirst
        (
            playerSelectedMove.mName,
            opponentSelectedMove.mName,
            activePlayerPokemon.mSpeedEncounterStage, 
            activeOpponentPokemon.mSpeedEncounterStage, 
            activePlayerPokemon.mSpeed,
            activeOpponentPokemon.mSpeed
        )
    )
    {
        encounterStateComponent.mIsOpponentsTurn = true;
        encounterStateComponent.mLastMoveSelected = opponentSelectedMove.mName;
    }
    else
    {
        //TODO: move PP check
        playerSelectedMove.mPowerPointsLeft--;
        encounterStateComponent.mLastMoveSelected = playerSelectedMove.mName;
    }
}

void RoundStructureCalculationEncounterFlowState::VUpdate(const float)
{        
    CompleteAndTransitionTo<DamageCalculationEncounterFlowState>();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
