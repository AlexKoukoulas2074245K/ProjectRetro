//
//  OverworldUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OverworldUtils.h"
#include "../components/ActiveLevelSingletonComponent.h"
#include "../components/WarpConnectionsSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/components/TransformComponent.h"
#include "LevelUtils.h"
#include "../components/LevelModelComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void DestroyOverworldModelNpcAndEraseTileInfo(const TileCoords& coords, ecs::World& world)
{
    // Since model and npc attributes are two separate entities (and they should be, as 
    // there can be a hidden item on top of a model for instance), all entities on the
    // given coords should be destroyed
    const auto& activeEntities = world.GetActiveEntities();
    for (const auto& entityId : activeEntities)
    {
        if (world.HasComponent<MovementStateComponent>(entityId))                
        {
            const auto& movementStateComponent = world.GetComponent<MovementStateComponent>(entityId);
            if (movementStateComponent.mCurrentCoords == coords)
            {
                world.DestroyEntity(entityId);
            }
        }
    }

    // Also clear tile occupier info
    const auto& activeLevelComponent = world.GetSingletonComponent<ActiveLevelSingletonComponent>();
    auto& levelModelComponent        = world.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, world));

    GetTile(coords, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = ecs::NULL_ENTITY_ID;
    GetTile(coords, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NONE;
}

void DestroyOverworldNpcEntityAndEraseTileInfo(const ecs::EntityId entityId, ecs::World& world)
{
    const auto& activeLevelComponent = world.GetSingletonComponent<ActiveLevelSingletonComponent>();
    auto& levelModelComponent        = world.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, world));

    auto& movementStateComponent = world.GetComponent<MovementStateComponent>(entityId);

    GetTile(movementStateComponent.mCurrentCoords.mCol, movementStateComponent.mCurrentCoords.mRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = ecs::NULL_ENTITY_ID;
    GetTile(movementStateComponent.mCurrentCoords.mCol, movementStateComponent.mCurrentCoords.mRow, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NONE;

    world.DestroyEntity(entityId);
}

ecs::EntityId FindEntityAtLevelCoords(const TileCoords& coords, const ecs::World& world)
{
    const auto entityPosition = TileCoordsToPosition(coords.mCol, coords.mRow);

    const auto& activeEntities = world.GetActiveEntities();

    const auto findIter = std::find_if(activeEntities.cbegin(), activeEntities.cend(), [&world, &entityPosition](const ecs::EntityId& entityId) 
    {
        if (world.HasComponent<TransformComponent>(entityId))
        {
            const auto& transformComponent = world.GetComponent<TransformComponent>(entityId);
            if
            (
                math::Abs(entityPosition.x - transformComponent.mPosition.x) < 0.01f &&
                math::Abs(entityPosition.y - transformComponent.mPosition.y) < 0.01f &&
                math::Abs(entityPosition.z - transformComponent.mPosition.z) < 0.01f
            )
            {
                return true;
            }            
        }

        return false;
    });

    if (findIter != activeEntities.cend()) return *findIter;
    return ecs::NULL_ENTITY_ID;
}

void SetCurrentPokeCenterAsHome(ecs::World& world)
{
    const auto& activeLevelComponent          = world.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& townMapLocationsDataComponent = world.GetSingletonComponent<TownMapLocationDataSingletonComponent>();
    const auto& warpConnectionsComponent      = world.GetSingletonComponent<WarpConnectionsSingletonComponent>();
    auto& playerStateComponent                = world.GetSingletonComponent<PlayerStateSingletonComponent>();

    playerStateComponent.mHomeLevelName = townMapLocationsDataComponent.mIndoorLocationsToOwnerLocations.at(activeLevelComponent.mActiveLevelNameId);

    // Find warp from town to this poke center and extract tile coords of entrance to center
    for (const auto& warpEntry : warpConnectionsComponent.mWarpConnections)
    {
        const auto& fromWarp = warpEntry.first;
        const auto& toWarp   = warpEntry.second;

        if (fromWarp.mLevelName == playerStateComponent.mHomeLevelName && toWarp.mLevelName == activeLevelComponent.mActiveLevelNameId)
        {
            playerStateComponent.mHomeLevelOccupiedCol = fromWarp.mTileCoords.mCol;
            playerStateComponent.mHomeLevelOccupiedRow = fromWarp.mTileCoords.mRow - 1;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
