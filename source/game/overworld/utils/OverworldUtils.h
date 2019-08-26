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
#include "../../overworld/components/NpcAiComponent.h"

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

inline ecs::EntityId GetNpcEntityIdFromLevelIndex(const int npcLevelIndex, const ecs::World& world)
{
    const auto& activeEntities = world.GetActiveEntities();
    for (const auto& entityId : activeEntities)
    {
        if
        (
            world.HasComponent<NpcAiComponent>(entityId) &&
            world.GetComponent<NpcAiComponent>(entityId).mLevelIndex == npcLevelIndex
        )
        {
            return entityId;
        }
    }
    
    return ecs::NULL_ENTITY_ID;
}

inline int GetNpcLevelIndexFromEntityId(const ecs::EntityId npcEntityId, const ecs::World& world)
{
    const auto& activeEntities = world.GetActiveEntities();
    for (const auto& entityId : activeEntities)
    {
        if (entityId == npcEntityId)
        {
            assert(world.HasComponent<NpcAiComponent>(npcEntityId) && "Requested level index from entity with no NpcAiComponent");
        }
        
        const auto& npcAiComponent = world.GetComponent<NpcAiComponent>(npcEntityId);
        return npcAiComponent.mLevelIndex;
    }
    
    assert(false && "Requested level index from a non-existent entity");
    return -1;
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

inline bool IsAnyNpcEngagedInCombat(const ecs::World& world)
{
    const auto& activeEntities = world.GetActiveEntities();
    for (const auto& entityId : activeEntities)
    {
        if
        (
            world.HasComponent<NpcAiComponent>(entityId) &&
            world.GetComponent<NpcAiComponent>(entityId).mIsEngagedInCombat
        )
        {
            return true;
        }
    }
    
    return false;
}

template<class FlowStateType>
inline void StartOverworldFlowState(ecs::World& world)
{
    auto& overworldFlowStateComponent = world.GetSingletonComponent<OverworldFlowStateSingletonComponent>();
    
    overworldFlowStateComponent.mFlowHasJustFinished = false;
    overworldFlowStateComponent.mFlowStateManager.SetActiveFlowState(std::make_unique<FlowStateType>(world));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* OverworldUtils_h */
