//
//  LearnNewMoveFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "LearnNewMoveFlowState.h"
#include "TrainerBattleWonEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/utils/PokemonUtils.h"
#include "../../common/utils/PokemonMoveUtils.h"
#include "../../common/utils/TextboxUtils.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

LearnNewMoveFlowState::LearnNewMoveFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];
    const auto& moveStats            = GetMoveStats(activePlayerPokemon.mMoveToBeLearned, world);
    
    activePlayerPokemon.mMoveSet[activePlayerPokemon.mMovesetIndexForNewMove] = std::make_unique<PokemonMoveStats>
    (
        moveStats.mName,
        moveStats.mType,
        moveStats.mEffect,
        moveStats.mPower,
        moveStats.mAccuracy,
        moveStats.mTotalPowerPoints
    );
    
    DestroyActiveTextbox(mWorld);
    
    const auto mainChatboxEntityId = CreateChatbox(world);
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        activePlayerPokemon.mName.GetString() + " learned#" + moveStats.mName.GetString() + "!#+END",
        mWorld
    );
}

void LearnNewMoveFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        CompleteAndTransitionTo<TrainerBattleWonEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
