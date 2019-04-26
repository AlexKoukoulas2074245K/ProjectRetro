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
#include "../components/CameraSingletonComponent.h"
#include "../components/WindowSingletonComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/components/PlayerTagComponent.h"
#include "../../common/utils/MathUtils.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float CameraControlSystem::CAMERA_Z_DISTANCE_FROM_PLAYER  = 19.0f;

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
    auto& cameraComponent           = mWorld.GetSingletonComponent<CameraSingletonComponent>();
    
    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto& focusedTransformComponent = mWorld.GetComponent<TransformComponent>(entityId);
            
            cameraComponent.mFocusPosition = focusedTransformComponent.mPosition;
            cameraComponent.mPosition.x    = focusedTransformComponent.mPosition.x;
            cameraComponent.mPosition.z    = focusedTransformComponent.mPosition.z + CAMERA_Z_DISTANCE_FROM_PLAYER;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
