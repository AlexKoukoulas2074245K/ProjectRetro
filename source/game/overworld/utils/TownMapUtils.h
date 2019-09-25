//
//  TownMapUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 24/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef TownMapUtils_h
#define TownMapUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/utils/StringUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class TownMapIconType
{
    PLAYER_ICON,
    CURSOR_ICON
};

class TownMapLocationDataSingletonComponent;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId LoadAndCreateTownMapBackground
(
    ecs::World& world
);

ecs::EntityId LoadAndCreateTownMapIconAtLocation
(
    const TownMapIconType iconType, 
    const StringId location,
    ecs::World& world
);

void LoadAndPopulateTownMapLocationData
(
    TownMapLocationDataSingletonComponent&
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TownMapUtils_h */
