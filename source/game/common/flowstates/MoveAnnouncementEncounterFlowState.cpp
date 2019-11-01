//
//  MoveAnnouncementEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MoveAnnouncementEncounterFlowState.h"
#include "MoveAnimationEncounterFlowState.h"
#include "MoveEffectivenessTextEncounterFlowState.h"
#include "MoveMissEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonMoveUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float MoveAnnouncementEncounterFlowState::MOVE_MISS_TRANSITION_DELAY = 0.3f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MoveAnnouncementEncounterFlowState::MoveAnnouncementEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto mainChatboxEntityId = CreateChatbox(world);

    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& selectedMove         = encounterStateComponent.mLastMoveSelected;
    
    const auto& attackingPokemon = encounterStateComponent.mIsOpponentsTurn ?
        *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex]:
        *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];

    encounterStateComponent.mViewObjects.mBattleAnimationTimer = nullptr;
    
    if (encounterStateComponent.mIsOpponentsTurn)
    {
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            "Enemy " + attackingPokemon.mName.GetString() + "#used " +  selectedMove.GetString() + "!+FREEZE",
            mWorld
        );
    }
    else
    {
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            attackingPokemon.mName.GetString() + "#used " +  selectedMove.GetString() + "!+FREEZE",
            mWorld
        );
    }
}

void MoveAnnouncementEncounterFlowState::VUpdate(const float dt)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();    
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& selectedMoveStats = GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld);
    const auto& defendingPokemon = encounterStateComponent.mIsOpponentsTurn ?
        *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex] :
        *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    auto effectivenessFactor = GetTypeEffectiveness(selectedMoveStats.mType, defendingPokemon.mBaseSpeciesStats.mFirstType, mWorld);

    if (defendingPokemon.mBaseSpeciesStats.mSecondType != StringId())
    {
        effectivenessFactor *= GetTypeEffectiveness(selectedMoveStats.mType, defendingPokemon.mBaseSpeciesStats.mSecondType, mWorld);
    }

    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        if (encounterStateComponent.mViewObjects.mBattleAnimationTimer == nullptr)
        {
            encounterStateComponent.mViewObjects.mBattleAnimationTimer = std::make_unique<Timer>(MOVE_MISS_TRANSITION_DELAY);
        }
        else
        {
            encounterStateComponent.mViewObjects.mBattleAnimationTimer->Update(dt);
            if (encounterStateComponent.mViewObjects.mBattleAnimationTimer->HasTicked())
            {
                encounterStateComponent.mViewObjects.mBattleAnimationTimer = nullptr;
                
                if (encounterStateComponent.mLastMoveMiss)
                {
                    CompleteAndTransitionTo<MoveMissEncounterFlowState>();
                }
                else if (effectivenessFactor < 0.1f || encounterStateComponent.mMoveHadNoEffect)
                {
                    CompleteAndTransitionTo<MoveEffectivenessTextEncounterFlowState>();
                }
                else if (encounterStateComponent.mNothingHappenedFromMoveExecution)
                {
                    CompleteAndTransitionTo<MoveEffectivenessTextEncounterFlowState>();
                }
                else
                {
                    CompleteAndTransitionTo<MoveAnimationEncounterFlowState>();
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

