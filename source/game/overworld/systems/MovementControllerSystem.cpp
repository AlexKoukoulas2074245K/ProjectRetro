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
#include "../components/ActiveLevelSingletonComponent.h"
#include "../components/LevelModelComponent.h"
#include "../components/MovementStateComponent.h"
#include "../components/WarpConnectionsSingletonComponent.h"
#include "../utils/LevelUtils.h"
#include "../utils/MovementUtils.h"
#include "../../common/GameConstants.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/components/PlayerTagComponent.h"
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

void MovementControllerSystem::VUpdateAssociatedComponents(const float dt) const
{
    auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    auto& levelModelComponent  = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));

    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (ShouldProcessEntity(entityId))
        {            
            const auto& directionComponent = mWorld.GetComponent<DirectionComponent>(entityId);
            auto& transformComponent       = mWorld.GetComponent<TransformComponent>(entityId);
            auto& movementStateComponent   = mWorld.GetComponent<MovementStateComponent>(entityId);
                        
            if (movementStateComponent.mMoving == false)
            {
                continue;
            }            

            const auto& currentTileCoords = movementStateComponent.mCurrentCoords;
            
            auto& currentTile = levelModelComponent.mLevelTilemap.at(currentTileCoords.mRow).at(currentTileCoords.mCol);

            const auto targetTileCoords = GetNeighborTileCoords(currentTileCoords, directionComponent.mDirection);
            
            // Bounds check
            if 
            (   
                targetTileCoords.mCol < 0 ||
                targetTileCoords.mRow < 0 ||
                targetTileCoords.mCol >= levelModelComponent.mCols ||
                targetTileCoords.mRow >= levelModelComponent.mRows
            )
            {
                movementStateComponent.mMoving = false;
                continue;
            }

            // Safe to now get the actual target tile            
            auto& targetTile  = GetTile(targetTileCoords, levelModelComponent.mLevelTilemap);

            // Occupier checks
            if (targetTile.mTileTrait == TileTrait::SOLID)
            {
                movementStateComponent.mMoving = false;
                continue;
            }
            
            if (targetTile.mTileOccupierType == TileOccupierType::NPC && targetTile.mTileOccupierEntityId != entityId)
            {
                movementStateComponent.mMoving = false;
                continue;
            }

            if (targetTile.mTileOccupierType == TileOccupierType::PLAYER && targetTile.mTileOccupierEntityId != entityId)
            {
                movementStateComponent.mMoving = false;
                continue;
            }

            // Clear occupier status of the current tile
            currentTile.mTileOccupierEntityId = ecs::NULL_ENTITY_ID;
            currentTile.mTileOccupierType     = TileOccupierType::NONE;

            // Set occupier status on the target tile
            const auto hasPlayerTag = mWorld.HasComponent<PlayerTagComponent>(entityId);
            targetTile.mTileOccupierEntityId = entityId;
            targetTile.mTileOccupierType     = hasPlayerTag ? TileOccupierType::PLAYER : TileOccupierType::NPC;

            // Move the transform to target by a tick
            const auto moveOutcome = MoveToTargetPosition
            (
                TileCoordsToPosition(targetTileCoords),
                CHARACTER_MOVEMENT_SPEED, 
                dt, 
                transformComponent.mPosition
            );

            // Target position reached in this frame
            if (moveOutcome == MoveOutcome::COMPLETED)
            {
                movementStateComponent.mMoving        = false;
                transformComponent.mPosition          = TileCoordsToPosition(targetTileCoords);
                movementStateComponent.mCurrentCoords = targetTileCoords;

                // If the player steps on a door or other warp, mark the event in the global WarpConnectionsComponent
                if (targetTile.mTileTrait == TileTrait::WARP && hasPlayerTag)
                {
                    mWorld.GetSingletonComponent<WarpConnectionsSingletonComponent>().mHasPendingWarpConnection = true;                    
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
