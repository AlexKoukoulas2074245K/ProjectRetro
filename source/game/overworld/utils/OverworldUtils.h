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
#include "../components/OverworldFlowStateSingletonComponent.h"
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

inline Direction GetDirectionFacingDirection(const Direction direction)
{
    return static_cast<Direction>((static_cast<int>(direction) + 2) % 4);
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

inline bool IsAnyOverworldFlowCurrentlyRunning(const ecs::World& world)
{
    return world.GetSingletonComponent<OverworldFlowStateSingletonComponent>().mFlowStateManager.HasActiveFlowState();
}

template<class FlowStateType>
inline void StartOverworldFlowState(ecs::World& world)
{
    world.GetSingletonComponent<OverworldFlowStateSingletonComponent>().mFlowStateManager.SetActiveFlowState(std::make_unique<FlowStateType>(world));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* OverworldUtils_h */
