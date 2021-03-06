//
//  LevelModelComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 03/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef LevelModelComponent_h
#define LevelModelComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/GameConstants.h"
#include "../../common/utils/StringUtils.h"
#include "../../rendering/utils/Colors.h"

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
    NONE,
    SOLID,
    WARP,
    NO_ANIM_WARP,
    PRESS_WARP,
    ENCOUNTER,
    JUMPING_LEDGE_BOT,
    JUMPING_LEDGE_LEFT,
    JUMPING_LEDGE_RIGHT,
    SEA_TILE_EDGE,
    FLOW_TRIGGER,
    CUTTABLE_TREE,
    PUSHABLE_ROCK

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct Tile
{
    TileTrait mTileTrait                = TileTrait::NONE;
    TileOccupierType mTileOccupierType  = TileOccupierType::NONE;
    ecs::EntityId mTileOccupierEntityId = ecs::NULL_ENTITY_ID;
};

struct WildEncounterInfo
{
    StringId mPokemonName;
    int mPokemonLevel;
    int mRate = 0;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

using LevelTilemap = std::vector<std::vector<Tile>>;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class LevelModelComponent final: public ecs::IComponent
{
public:    
     LevelTilemap mLevelTilemap;
     std::vector<WildEncounterInfo> mWildEncounters;
     StringId mLevelName              = StringId();
     StringId mLevelMusicTrackName    = StringId();
     glm::vec4 mLevelColor            = colors::PALLET_COLOR;
     ecs::EntityId mGroundLayerEntity = ecs::NULL_ENTITY_ID;
     int mEncounterDensity            = 0;
     int mCols                        = 0;
     int mRows                        = 0;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* LevelModelComponent_h */
