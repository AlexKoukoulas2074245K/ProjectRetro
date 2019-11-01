//
//  TrainerBattleWonEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TrainerBattleWonEncounterFlowState.h"
#include "TrainerBattleWonDefeatedTextEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../utils/TrainerUtils.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string TrainerBattleWonEncounterFlowState::GYM_BATTLE_WON_MUSIC_TRACK_NAME     = "gym_battle_won";
const std::string TrainerBattleWonEncounterFlowState::TRAINER_BATTLE_WON_MUSIC_TRACK_NAME = "trainer_battle_won";

const glm::vec3 TrainerBattleWonEncounterFlowState::OPPONENT_TRAINER_INIT_POS   = glm::vec3(0.95f, 0.61f, 0.1f);
const glm::vec3 TrainerBattleWonEncounterFlowState::OPPONENT_TRAINER_TARGET_POS = glm::vec3(0.38f, 0.61f, 0.3f);
const glm::vec3 TrainerBattleWonEncounterFlowState::SPRITE_SCALE                = glm::vec3(0.49f, 0.49f, 1.0f);

const float TrainerBattleWonEncounterFlowState::SPRITE_ANIMATION_SPEED = 1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TrainerBattleWonEncounterFlowState::TrainerBattleWonEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& trainerInfo          = GetTrainerInfo(encounterStateComponent.mOpponentTrainerSpeciesName, mWorld);

    // Means that we have already been through this
    if (encounterStateComponent.mHasPokemonEvolvedInBattle)
    {
        return;
    }

    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        DestroyActiveTextbox(mWorld);
    }

    const auto mainChatboxEntityId = CreateChatbox(world);
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        playerStateComponent.mPlayerTrainerName.GetString() + " defeated#" + encounterStateComponent.mOpponentTrainerName.GetString() + "!#+END",
        mWorld
    );
    
    encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId = LoadAndCreateTrainerSprite
    (
        trainerInfo.mTextureAtlasCol,
        trainerInfo.mTextureAtlasRow,
        OPPONENT_TRAINER_INIT_POS,
        SPRITE_SCALE,
        mWorld
    );

    SoundService::GetInstance().PlayMusic
    (
        encounterStateComponent.mIsGymLeaderBattle ? 
        GYM_BATTLE_WON_MUSIC_TRACK_NAME : 
        TRAINER_BATTLE_WON_MUSIC_TRACK_NAME,
        false
    );
}

void TrainerBattleWonEncounterFlowState::VUpdate(const float dt)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (encounterStateComponent.mHasPokemonEvolvedInBattle)
    {
        encounterStateComponent.mEncounterJustFinished = true;
        return;
    }

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {                
        auto& opponentTrainerSpriteTransformComponent = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);        
        opponentTrainerSpriteTransformComponent.mPosition.x -= SPRITE_ANIMATION_SPEED * dt;

        if (opponentTrainerSpriteTransformComponent.mPosition.x < OPPONENT_TRAINER_TARGET_POS.x)
        {
            opponentTrainerSpriteTransformComponent.mPosition.x = OPPONENT_TRAINER_TARGET_POS.x;            
            
            CompleteAndTransitionTo<TrainerBattleWonDefeatedTextEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
