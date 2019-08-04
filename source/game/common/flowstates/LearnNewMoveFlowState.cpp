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
#include "NextOpponentPokemonCheckEncounterFlowState.h"
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
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& activePlayerPokemon     = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];
    const auto& moveStats         = GetMoveStats(activePlayerPokemon.mMoveToBeLearned, world);
    
    playerStateComponent.mLeveledUpPokemonRosterIndex = -1;
    
    activePlayerPokemon.mMoveSet[activePlayerPokemon.mMovesetIndexForNewMove] = std::make_unique<PokemonMoveStats>
    (
        moveStats.mName,
        moveStats.mType,
        moveStats.mEffect,
        moveStats.mPower,
        moveStats.mAccuracy,
        moveStats.mTotalPowerPoints
    );
    
    activePlayerPokemon.mMoveToBeLearned = StringId();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        DestroyActiveTextbox(mWorld);
    }
    
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
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& playerStateComponent          = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        playerStateComponent.mLeveledUpPokemonRosterIndex = -1;
        if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
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
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
