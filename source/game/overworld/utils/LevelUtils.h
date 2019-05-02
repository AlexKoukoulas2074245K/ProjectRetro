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

inline LevelTilemap InitializeTilemapWithDimensions(const int cols, const int rows)
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
    return glm::vec3(coords.mCol * OVERWORLD_TILE_SIZE, 0.0f, coords.mRow * OVERWORLD_TILE_SIZE);
}

inline glm::vec3 TileCoordsToPosition(const int col, const int row)
{
    return glm::vec3(col * OVERWORLD_TILE_SIZE, 0.0f, row * OVERWORLD_TILE_SIZE);
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

inline TileCoords GetNeighborTileCoords(const TileCoords& coords, const Direction direction)
{
    switch (direction)
    {
        case Direction::EAST:  return TileCoords(coords.mCol + 1, coords.mRow);
        case Direction::NORTH: return TileCoords(coords.mCol,     coords.mRow + 1);
        case Direction::SOUTH: return TileCoords(coords.mCol,     coords.mRow - 1);
        case Direction::WEST:  return TileCoords(coords.mCol - 1, coords.mRow);
    }

    return TileCoords();
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

inline ecs::EntityId GetOverworldPlayerEntityId(const ecs::World& world)
{
    const auto& activeEntities = world.GetActiveEntities();
    for (const auto& entityId : activeEntities)
    {
        if (world.HasComponent<PlayerTagComponent>(entityId))
        {
            return entityId;
        }
    }

    return ecs::NULL_ENTITY_ID;
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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* LevelUtils_h */
