//
//  OverworldUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 03/05/2019.
//

#ifndef OverworldUtils_h
#define OverworldUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../OverworldConstants.h"
#include "../../common/components/PlayerTagComponent.h"
#include "../../common/GameConstants.h"
#include "../../ECS.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

inline StringId GetDirectionAnimationName(const Direction direction)
{
    switch (direction)
    {
        case Direction::NORTH: return NORTH_ANIMATION_NAME_ID;
        case Direction::SOUTH: return SOUTH_ANIMATION_NAME_ID;
        case Direction::WEST:  return WEST_ANIMATION_NAME_ID;
        case Direction::EAST:  return EAST_ANIMATION_NAME_ID;
    }
    
    return StringId();
}

inline ecs::EntityId GetPlayerEntityId(const ecs::World& world)
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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* OverworldUtils_h */
