//
//  NewMovesCheckFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "NewMovesCheckFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonMoveUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

NewMovesCheckFlowState::NewMovesCheckFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster.front();    
    
    activePlayerPokemon.mMoveToBeLearned = StringId();

    for (const auto& moveLearnInfo : activePlayerPokemon.mBaseStats.mLearnset)
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
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster.front();

    if (activePlayerPokemon.mMoveToBeLearned != StringId())
    {
        const auto firstUnusedIndex = FindFirstUnusedMoveIndex(activePlayerPokemon.mMoveSet);

        // Not enough space for new move
        if (firstUnusedIndex == 4)
        {
            //CompleteAndTransitionTo<DeleteMoveToInsertNewOneFlowState>();
			// AAAAAAAAA is
			// trying to learn
			// Quick Attack!
			
			// But, AAAA
			// can't learn more 
			// than 4 moves!
			
			// Delete an older
			// move to make room
			// for Quick Attack?
			
			// YES-NO
			
			// (YES)
			// Which move should 
			// be forgotten?+FREEZE
			// MOVE SELECTION ABOVE
			// 1, 2 PAUSE and... Poof!+END
			
			// AAAAA forgot
			// GROWL!
			
			// And...
			
			// AAAAA learned
			// QUICK ATTACK!+END
			
			
			// (NO)
			// Abandon learning
			// QUICK ATTACK?+FREEZE
			
			// YES-NO
			
			// (NO)
			// AAAA is trying to learn...
			
			// (YES)
			// AAAAAAAAA
			// did not learn
			// QUICK ATTACK!+END
        }
        else
        {
            //CompleteAndTransitionTo<LearnNewMoveFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
