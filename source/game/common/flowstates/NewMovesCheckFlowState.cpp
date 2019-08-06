//
//  NewMovesCheckFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "EvolutionTextFlowState.h"
#include "NewMovesCheckFlowState.h"
#include "LearnNewMoveFlowState.h"
#include "NextOpponentPokemonCheckEncounterFlowState.h"
#include "TrainerBattleWonEncounterFlowState.h"
#include "FullMovesetIntroTextFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonUtils.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

NewMovesCheckFlowState::NewMovesCheckFlowState(ecs::World& world)
    : BaseFlowState(world)
{    
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];
    
    activePlayerPokemon.mMoveToBeLearned = StringId();

    for (const auto& moveLearnInfo : activePlayerPokemon.mBaseSpeciesStats.mLearnset)
    {
        if 
        (
            moveLearnInfo.mLevelLearned == activePlayerPokemon.mLevel &&
            DoesMovesetHaveMove(moveLearnInfo.mMoveName, activePlayerPokemon.mMoveSet) == false
        )
        {
            activePlayerPokemon.mMoveToBeLearned = moveLearnInfo.mMoveName;
        }
    }
}

void NewMovesCheckFlowState::VUpdate(const float)
{ 
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& activePlayerPokemon     = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];

    // No new moves to be learned
    if (activePlayerPokemon.mMoveToBeLearned == StringId())
    {
        playerStateComponent.mLeveledUpPokemonRosterIndex = -1;
        if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
        {
            CompleteAndTransitionTo<EvolutionTextFlowState>();
        }
        else if (encounterStateComponent.mActiveEncounterType == EncounterType::TRAINER)
        {
            if (GetFirstNonFaintedPokemonIndex(encounterStateComponent.mOpponentPokemonRoster) != encounterStateComponent.mOpponentPokemonRoster.size())
            {
                CompleteAndTransitionTo<NextOpponentPokemonCheckEncounterFlowState>();
            }
            else
            {
                CompleteAndTransitionTo<TrainerBattleWonEncounterFlowState>();
            }
        }
        else
        {
            //TODO: Continue with overworld
        }
    }
    else
    {
        const auto firstUnusedIndex = FindFirstUnusedMoveIndex(activePlayerPokemon.mMoveSet);

        // Not enough space for new move
        if (firstUnusedIndex == 4)
        {
            CompleteAndTransitionTo<FullMovesetIntroTextFlowState>();
        }
        else
        {
            playerStateComponent.mLeveledUpPokemonRosterIndex = -1;
            activePlayerPokemon.mMovesetIndexForNewMove = firstUnusedIndex;
            CompleteAndTransitionTo<LearnNewMoveFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
