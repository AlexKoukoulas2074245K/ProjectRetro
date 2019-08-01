//
//  MoveOpponentShakeEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MoveShakeEncounterFlowState.h"
#include "HealthDepletionEncounterFlowState.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../common/components/TransformComponent.h"
#include "../../encounter/components/EncounterShakeSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../rendering/components/CameraSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 MoveShakeEncounterFlowState::ENCOUNTER_LEFT_EDGE_POSITION  = glm::vec3(-0.937f, 0.0f, -1.0f);
const glm::vec3 MoveShakeEncounterFlowState::ENCOUNTER_RIGHT_EDGE_POSITION = glm::vec3(0.962f, 0.0f, -1.0f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MoveShakeEncounterFlowState::MoveShakeEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    DetermineShakeTypeToBeInitiated();
}

void MoveShakeEncounterFlowState::VUpdate(const float)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& cameraComponent         = mWorld.GetSingletonComponent<CameraSingletonComponent>();
    const auto& shakeComponent          = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();

    // Wait until Shake is finished
    if (shakeComponent.mActiveShakeType == ShakeType::NONE)
    {
        CompleteAndTransitionTo<HealthDepletionEncounterFlowState>();
    }
    else
    {
        // Make level edges immune to global screen offsets to achieve the desired cutoff
        auto& levelLeftEdgeTransformComponent  = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mLevelLeftEdgeEntityId);
        auto& levelRightEdgeTransformComponent = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mLevelRightEdgeEntityId);
        
        levelLeftEdgeTransformComponent.mPosition  = ENCOUNTER_LEFT_EDGE_POSITION - cameraComponent.mGlobalScreenOffset;
        levelRightEdgeTransformComponent.mPosition = ENCOUNTER_RIGHT_EDGE_POSITION - cameraComponent.mGlobalScreenOffset;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void MoveShakeEncounterFlowState::DetermineShakeTypeToBeInitiated() const
{
    const auto& encounterComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& lastMoveUsedStats  = GetMoveStats(encounterComponent.mLastMoveSelected, mWorld);
    const auto isOpponentsTurn     = encounterComponent.mIsOpponentsTurn;
    auto& shakeComponent           = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();

    shakeComponent.mShakeProgressionStep = 0;
    
    if (isOpponentsTurn == false)
    {
        if (lastMoveUsedStats.mEffect == StringId())
        {
            shakeComponent.mActiveShakeType = ShakeType::OPPONENT_POKEMON_BLINK;
        }
        else if (lastMoveUsedStats.mEffect != StringId() && lastMoveUsedStats.mPower == 0)
        {
            shakeComponent.mActiveShakeType = ShakeType::OPPONENT_POKEMON_LONG_HORIZONTAL_SHAKE;
        }
        else if (lastMoveUsedStats.mEffect != StringId() && lastMoveUsedStats.mPower > 0)
        {
            shakeComponent.mActiveShakeType = ShakeType::OPPONENT_POKEMON_SHORT_HORIZONTAL_SHAKE;
        }
    }
    else
    {
        if (lastMoveUsedStats.mEffect == StringId())
        {
            shakeComponent.mActiveShakeType = ShakeType::PLAYER_POKEMON_VERTICAL_SHAKE;
        }
        else if (lastMoveUsedStats.mEffect != StringId() && lastMoveUsedStats.mPower == 0)
        {
            shakeComponent.mActiveShakeType = ShakeType::PLAYER_POKEMON_LONG_HORIZONTAL_SHAKE;
        }
        else if (lastMoveUsedStats.mEffect != StringId() && lastMoveUsedStats.mPower > 0)
        {
            shakeComponent.mActiveShakeType = ShakeType::PLAYER_RAPID_LONG_HORIZONTAL_SHAKE;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
