//
//  StatusChangeShakeEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "StatusChangeShakeEncounterFlowState.h"
#include "StatusChangeTextEncounterFlowState.h"
#include "../components/TransformComponent.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/components/EncounterShakeSingletonComponent.h"
#include "../../rendering/components/CameraSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 StatusChangeShakeEncounterFlowState::ENCOUNTER_LEFT_EDGE_POSITION  = glm::vec3(-0.937f, 0.0f, -1.0f);
const glm::vec3 StatusChangeShakeEncounterFlowState::ENCOUNTER_RIGHT_EDGE_POSITION = glm::vec3(0.962f, 0.0f, -1.0f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

StatusChangeShakeEncounterFlowState::StatusChangeShakeEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    DetermineStatusShakeSequence();
}

void StatusChangeShakeEncounterFlowState::VUpdate(const float)
{  
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& cameraComponent = mWorld.GetSingletonComponent<CameraSingletonComponent>();
    const auto& shakeComponent = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();

    // Wait until Shake is finished
    if (shakeComponent.mActiveShakeType == ShakeType::NONE)
    {
        CompleteAndTransitionTo<StatusChangeTextEncounterFlowState>();
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

void StatusChangeShakeEncounterFlowState::DetermineStatusShakeSequence() const
{
    const auto& encounterComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();    
    const auto& lastMoveUsedStats  = GetMoveStats(encounterComponent.mLastMoveSelected, mWorld);        
    auto& shakeComponent           = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();    

    if 
    (
        IsMoveNonShake(lastMoveUsedStats.mName) ||
        (
            encounterComponent.mPendingStatusToBeAppliedToPlayerPokemon == PokemonStatus::CONFUSED ||
            encounterComponent.mPendingStatusToBeAppliedToOpponentPokemon == PokemonStatus::CONFUSED
        )
    )
    {
        return;
    }

    shakeComponent.mShakeProgressionStep = 0;

    if (encounterComponent.mPendingStatusToBeAppliedToPlayerPokemon != PokemonStatus::NORMAL)
    {
        shakeComponent.mActiveShakeType = ShakeType::PLAYER_POKEMON_STATUS_SHAKE;
    }
    else if (encounterComponent.mPendingStatusToBeAppliedToOpponentPokemon != PokemonStatus::NORMAL)
    {
        shakeComponent.mActiveShakeType = ShakeType::OPPONENT_POKEMON_STATUS_SHAKE;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

