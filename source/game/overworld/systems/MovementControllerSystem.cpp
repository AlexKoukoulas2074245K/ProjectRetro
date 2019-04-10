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
#include "../components/LevelTilemapComponent.h"
#include "../components/MovementStateComponent.h"
#include "../utils/LevelUtils.h"
#include "../utils/MovementUtils.h"
#include "../../common/GameConstants.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/components/PlayerTagComponent.h"
#include "../../common/components/TransformComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float MovementControllerSystem::CHARACTER_MOVEMENT_SPEED = 2 * OVERWORLD_TILE_SIZE;

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
    auto& levelTilemapComponent = mWorld.GetSingletonComponent<LevelTilemapComponent>();

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
            auto& currentTile = levelTilemapComponent.mLevelTilemap.at(currentTileCoords.mRow).at(currentTileCoords.mCol);

            const auto targetTileCoords = GetNeighborTileCoords(currentTileCoords, directionComponent.mDirection);
            
            // Bounds check
            if 
            (   
                targetTileCoords.mCol < 0 ||
                targetTileCoords.mRow < 0 ||
                targetTileCoords.mCol >= levelTilemapComponent.mCols ||
                targetTileCoords.mRow >= levelTilemapComponent.mRows
            )
            {
                movementStateComponent.mMoving = false;
                continue;
            }

            // Safe to now get the actual target tile            
            auto& targetTile  = GetTile(targetTileCoords, levelTilemapComponent.mLevelTilemap);

            // Occupier checks
            if (targetTile.mTileOccupierType == TileOccupierType::SOLID)
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
            targetTile.mTileOccupierEntityId = entityId;
            targetTile.mTileOccupierType     = mWorld.HasComponent<PlayerTagComponent>(entityId) ? TileOccupierType::PLAYER : TileOccupierType::NPC;

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
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////