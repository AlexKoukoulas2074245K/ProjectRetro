//
//  OpponentTrainerRetreatEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OpponentTrainerRetreatEncounterFlowState.h"
#include "OpponentTrainerPokemonSummonTextEncounterFlowState.h"
#include "../../common/components/TransformComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 OpponentTrainerRetreatEncounterFlowState::OPPONENT_SPRITE_TARGET_POS = glm::vec3(0.99f, 0.61f, 0.3f);

const float OpponentTrainerRetreatEncounterFlowState::SPRITE_ANIMATION_SPEED = 2.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OpponentTrainerRetreatEncounterFlowState::OpponentTrainerRetreatEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
}

void OpponentTrainerRetreatEncounterFlowState::VUpdate(const float dt)
{
    const auto& encounterStateComponent         = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& playerTrainerSpriteTransformComponent = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
    
    playerTrainerSpriteTransformComponent.mPosition.x += SPRITE_ANIMATION_SPEED * dt;
    if (playerTrainerSpriteTransformComponent.mPosition.x > OPPONENT_SPRITE_TARGET_POS.x)
    {
        playerTrainerSpriteTransformComponent.mPosition.x = OPPONENT_SPRITE_TARGET_POS.x;
        CompleteAndTransitionTo<OpponentTrainerPokemonSummonTextEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

