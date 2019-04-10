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

#include "../components/LevelGridComponent.h"
#include "../components/MovementStateComponent.h"
#include "../../Common/components/TransformComponent.h"
#include "../../common/GameConstants.h"
#include "../../common/utils/MathUtils.h"
#include "../../ECS.h"

#include <cstddef>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

inline LevelGrid InitializeLevelGridOfDimensions(const int cols, const int rows)
{
    LevelGrid result(rows);

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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* LevelUtils_h */
