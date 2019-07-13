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
#include "MoveMissEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"

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
        encounterStateComponent.mOpponentPokemonRoster.front() :
        playerStateComponent.mPlayerPokemonRoster.front();

    encounterStateComponent.mViewObjects.mBattleAnimationTimer = nullptr;
    
    //TODO: differentiate between summoning dialogs
    QueueDialogForTextbox
    (
        mainChatboxEntityId,        
        attackingPokemon->mName.GetString() + "#used " +  selectedMove.GetString() + "!+FREEZE",
        mWorld
    );
}

void MoveAnnouncementEncounterFlowState::VUpdate(const float dt)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

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

