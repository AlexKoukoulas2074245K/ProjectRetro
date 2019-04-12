//
//  LevelTilemapComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 03/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef LevelTilemapComponent_h
#define LevelTilemapComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/utils/StringUtils.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class TileOccupierType
{
    NONE, NPC, PLAYER
};

enum class TileTrait
{
    NONE, SOLID, WARP
};

struct Tile
{
    TileTrait mTileTrait                = TileTrait::NONE;
    TileOccupierType mTileOccupierType  = TileOccupierType::NONE;
    ecs::EntityId mTileOccupierEntityId = ecs::NULL_ENTITY_ID;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

using LevelTilemap = std::vector<std::vector<Tile>>;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class LevelTilemapComponent final: public ecs::IComponent
{
public:    
     StringId mLevelName;
     LevelTilemap mLevelTilemap;
     int mCols;
     int mRows;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* LevelTilemapComponent_h */
