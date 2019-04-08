//
//  MovementControllerSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 03/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MovementControllerSystem.h"
#include "../components/LevelGridComponent.h"
#include "../components/MovementStateComponent.h"
#include "../utils/LevelUtils.h"
#include "../../common/GameConstants.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/components/TransformComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float MovementControllerSystem::CHARACTER_MOVEMENT_SPEED = 4 * OVERWORLD_TILE_SIZE;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MovementControllerSystem::MovementControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<MovementStateComponent, DirectionComponent, TransformComponent>();
}

void MovementControllerSystem::VUpdate(const float) const
{
    const auto& levelGridComponent = mWorld.GetSingletonComponent<LevelGridComponent>();

    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto& directionComponent = mWorld.GetComponent<DirectionComponent>(entityId);
            auto& movementStateComponent   = mWorld.GetComponent<MovementStateComponent>(entityId);

            if (movementStateComponent.mMoving == false)
            {
                continue;
            }

            const auto targetTile = GetNeighborTileCoords(movementStateComponent.mCurrentCoords, directionComponent.mDirection);

            // Do bounds check
            // Do solid, other occupier checks etc
            // Call movement utils to move object
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
