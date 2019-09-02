//
//  DarkenedOpponentsIntroEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "DarkenedOpponentsIntroEncounterFlowState.h"
#include "OpponentIntroTextEncounterFlowState.h"
#include "../components/TransformComponent.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TrainerUtils.h"
#include "../../ECS.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../overworld/components/TransitionAnimationStateSingletonComponent.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 DarkenedOpponentsIntroEncounterFlowState::PLAYER_TRAINER_SPRITE_INIT_POS   = glm::vec3(0.9f, 0.06f, 0.1f);
const glm::vec3 DarkenedOpponentsIntroEncounterFlowState::PLAYER_TRAINER_SPRITE_TARGET_POS = glm::vec3(-0.39f, 0.06f, 0.1f);
const glm::vec3 DarkenedOpponentsIntroEncounterFlowState::OPPONENT_SPRITE_INIT_POS         = glm::vec3(-0.9f, 0.61f, 0.3f);
const glm::vec3 DarkenedOpponentsIntroEncounterFlowState::OPPONENT_SPRITE_TARGET_POS       = glm::vec3(0.38f, 0.61f, 0.3f);
const glm::vec3 DarkenedOpponentsIntroEncounterFlowState::SPRITE_SCALE                     = glm::vec3(0.49f, 0.49f, 1.0f);

const float DarkenedOpponentsIntroEncounterFlowState::SPRITE_ANIMATION_SPEED = 1.0f;

const int DarkenedOpponentsIntroEncounterFlowState::PLAYER_TRAINER_SPRITE_ATLAS_COL = 6;
const int DarkenedOpponentsIntroEncounterFlowState::PLAYER_TRAINER_SPRITE_ATLAS_ROW = 4;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

DarkenedOpponentsIntroEncounterFlowState::DarkenedOpponentsIntroEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    CreateEncounterOpponentsSprites();
    CreateEncounterEdges();
    mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>().mBlackAndWhiteModeEnabled = true;
}

void DarkenedOpponentsIntroEncounterFlowState::VUpdate(const float dt)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    auto& playerTrainerSpriteTransformComponent   = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);
    auto& opponentTrainerSpriteTransformComponent = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
    
    playerTrainerSpriteTransformComponent.mPosition.x   -= SPRITE_ANIMATION_SPEED * dt;
    opponentTrainerSpriteTransformComponent.mPosition.x += SPRITE_ANIMATION_SPEED * dt;
    
    if (playerTrainerSpriteTransformComponent.mPosition.x < PLAYER_TRAINER_SPRITE_TARGET_POS.x)
    {
        playerTrainerSpriteTransformComponent.mPosition.x   = PLAYER_TRAINER_SPRITE_TARGET_POS.x;
        opponentTrainerSpriteTransformComponent.mPosition.x = OPPONENT_SPRITE_TARGET_POS.x;
        
        mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>().mBlackAndWhiteModeEnabled = false;

        if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
        {
            SoundService::GetInstance().PlaySfx
            (
                "cries/" +
                GetFormattedPokemonIdString(encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex]->mBaseSpeciesStats.mId)
            );
        }
        
        CompleteAndTransitionTo<OpponentIntroTextEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void DarkenedOpponentsIntroEncounterFlowState::CreateEncounterOpponentsSprites() const
{    
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId = LoadAndCreateTrainerSprite
    (
        PLAYER_TRAINER_SPRITE_ATLAS_COL,
        PLAYER_TRAINER_SPRITE_ATLAS_ROW,
        PLAYER_TRAINER_SPRITE_INIT_POS,
        SPRITE_SCALE,
        mWorld
    );
    
    if (encounterStateComponent.mActiveEncounterType == EncounterType::TRAINER)
    {
        const auto& trainerInfo = GetTrainerInfo(encounterStateComponent.mOpponentTrainerSpeciesName, mWorld);
        
        encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId = LoadAndCreateTrainerSprite
        (
            trainerInfo.mTextureAtlasCol,
            trainerInfo.mTextureAtlasRow,
            OPPONENT_SPRITE_INIT_POS,
            SPRITE_SCALE,
            mWorld
        );
    }
    else if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
    {
        encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId = LoadAndCreatePokemonSprite
        (
            encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex]->mBaseSpeciesStats.mSpeciesName,
            true,
            OPPONENT_SPRITE_INIT_POS,
            SPRITE_SCALE,
            mWorld
        );
    }
    
    
    mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>().mBlackAndWhiteModeEnabled = true;
}

void DarkenedOpponentsIntroEncounterFlowState::CreateEncounterEdges() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    const auto& encounterEdgeEntityIds = LoadAndCreateLevelEdges(mWorld);

    encounterStateComponent.mViewObjects.mLevelLeftEdgeEntityId  = encounterEdgeEntityIds.first;
    encounterStateComponent.mViewObjects.mLevelRightEdgeEntityId = encounterEdgeEntityIds.second;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

