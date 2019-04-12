//
//  CameraControlSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "CameraControlSystem.h"
#include "../components/CameraComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/components/PlayerTagComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float CameraControlSystem::CAMERA_Y_ELEVATION_FROM_GROUND = 9.0f;
const float CameraControlSystem::CAMERA_Z_DISTANCE_FROM_PLAYER  = 6.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

CameraControlSystem::CameraControlSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<PlayerTagComponent>();
}

void CameraControlSystem::VUpdateAssociatedComponents(const float) const
{    
    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto& focusedTransformComponent = mWorld.GetComponent<TransformComponent>(entityId);
            auto& cameraComponent = mWorld.GetSingletonComponent<CameraComponent>();
            
            cameraComponent.mFocusPosition = focusedTransformComponent.mPosition;
            cameraComponent.mPosition.x    = focusedTransformComponent.mPosition.x;
            cameraComponent.mPosition.y    = CAMERA_Y_ELEVATION_FROM_GROUND;
            cameraComponent.mPosition.z    = focusedTransformComponent.mPosition.z - CAMERA_Z_DISTANCE_FROM_PLAYER;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
