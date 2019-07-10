//
//  OverworldConstants.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef OverworldConstants_h
#define OverworldConstants_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../common/utils/StringUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class OverworldPokemonSpriteType
{
    BALL, BEAST, BUG, DRAGON, FLYING, FOSSIL, GRASS, NORMAL, PIKACHU, WATER
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const StringId NORTH_ANIMATION_NAME_ID = StringId("north");
static const StringId SOUTH_ANIMATION_NAME_ID = StringId("south");
static const StringId WEST_ANIMATION_NAME_ID  = StringId("west");
static const StringId EAST_ANIMATION_NAME_ID  = StringId("east");

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* OverworldConstants_h */
