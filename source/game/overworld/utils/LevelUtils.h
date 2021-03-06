//
//  LevelUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef LevelUtils_h
#define LevelUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../components/LevelModelComponent.h"
#include "../components/LevelResidentComponent.h"
#include "../components/MovementStateComponent.h"
#include "../../common/components/PlayerTagComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/GameConstants.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/StringUtils.h"
#include "../../ECS.h"

#include <cstddef>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string INDOOR_LEVEL_NAME_PREFIX = "in_";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

inline bool IsLevelIndoors(const StringId levelNameId)
{    
    return StringStartsWith(levelNameId.GetString(), INDOOR_LEVEL_NAME_PREFIX);
}

inline LevelTilemap CreateTilemapWithDimensions(const int cols, const int rows)
{
    LevelTilemap result(rows);

    for (auto& row : result)
    {
        row.resize(cols);
    }

    return result;
}

inline glm::vec3 TileCoordsToPosition(const TileCoords& coords)
{
    return glm::vec3(coords.mCol * GAME_TILE_SIZE, 0.0f, coords.mRow * GAME_TILE_SIZE);
}

inline glm::vec3 TileCoordsToPosition(const int col, const int row)
{
    return glm::vec3(col * GAME_TILE_SIZE, 0.0f, row * GAME_TILE_SIZE);
}

inline const Tile& GetTile(const int col, const int row, const LevelTilemap& grid)
{
    return grid.at(row).at(col);
}

inline Tile& GetTile(const int col, const int row, LevelTilemap& grid)
{
    return grid.at(row).at(col);
}

inline const Tile& GetTile(const TileCoords& coords, const LevelTilemap& grid)
{
    return grid.at(coords.mRow).at(coords.mCol);
}

inline Tile& GetTile(const TileCoords& coords, LevelTilemap& grid)
{
    return grid.at(coords.mRow).at(coords.mCol);
}

inline TileCoords GetNeighborTileCoords(const TileCoords& coords, const Direction direction, const int numberOfTilesAway = 1)
{
    switch (direction)
    {
        case Direction::EAST:  return TileCoords(coords.mCol + numberOfTilesAway, coords.mRow);
        case Direction::NORTH: return TileCoords(coords.mCol,     coords.mRow + numberOfTilesAway);
        case Direction::SOUTH: return TileCoords(coords.mCol,     coords.mRow - numberOfTilesAway);
        case Direction::WEST:  return TileCoords(coords.mCol - numberOfTilesAway, coords.mRow);
    }

    return TileCoords();
}

inline const Tile& GetNeighborTile(const TileCoords& coords, const Direction direction, const LevelTilemap& grid, const int numberOfTilesAway = 1)
{
    const auto& tileCoords = GetNeighborTileCoords(coords, direction, numberOfTilesAway);
    return GetTile(tileCoords, grid);
}

inline Tile& GetNeighborTile(const TileCoords& coords, const Direction direction, LevelTilemap& grid, const int numberOfTilesAway = 1)
{
    const auto& tileCoords = GetNeighborTileCoords(coords, direction, numberOfTilesAway);
    return GetTile(tileCoords, grid);
}

inline StringId GetLevelNameFromId(const ecs::EntityId levelId, const ecs::World& world)
{
    const auto& activeEntities = world.GetActiveEntities();
    for (const auto& entityId : activeEntities)
    {
        if (entityId == levelId)
        {
            return world.GetComponent<LevelModelComponent>(levelId).mLevelName;
        }
    }

    return StringId();
}

inline ecs::EntityId GetLevelIdFromNameId(const StringId& levelNameId, const ecs::World& world)
{
    const auto& activeEntities = world.GetActiveEntities();
    for (const auto& entityId : activeEntities)
    {
        if 
        (
            world.HasComponent<LevelModelComponent>(entityId) &&
            world.GetComponent<LevelModelComponent>(entityId).mLevelName == levelNameId
        )
        {
            return entityId;
        }
    }
    
    return ecs::NULL_ENTITY_ID;
}

inline void DestroyLevel(const StringId levelNameId, ecs::World& world)
{
    for (const auto& entityId : world.GetActiveEntities())
    {
        if (world.HasComponent<LevelResidentComponent>(entityId))
        {
            if (world.GetComponent<LevelResidentComponent>(entityId).mLevelNameId == levelNameId)
            {
                world.DestroyEntity(entityId);
            }
        }
    }
    
    world.DestroyEntity(GetLevelIdFromNameId(levelNameId, world));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* LevelUtils_h */
