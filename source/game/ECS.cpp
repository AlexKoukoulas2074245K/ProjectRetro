//
//  ECS.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 26/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ECS.h"


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::World::World()
{
    mEntityComponentStore.reserve(ANTICIPATED_ENTITY_COUNT);
}

const std::vector<ecs::EntityId>& ecs::World::GetActiveEntities() const
{
    return mActiveEntitiesInFrame;
}

void ecs::World::Update(const float dt)
{
    RemoveEntitiesWithoutAnyComponents();
    CongregateActiveEntitiesInCurrentFrame();

    auto systemReverseIter = mSystems.rbegin();
    while (systemReverseIter != mSystems.rend())
    {        
        systemReverseIter->second->VUpdate(dt);
        systemReverseIter++;
    }
}

void ecs::World::RemoveEntity(const EntityId entityId)
{
    assert(mEntityComponentStore.count(entityId) != 0 &&
        "Entity does not exist in the world");

    mEntityComponentStore.at(entityId).clear();
}

ecs::ComponentMask ecs::World::CalculateEntityComponentUsageMask(const EntityId entityId) const
{
    ComponentMask componentUsageMask;
    const auto& componentMap = mEntityComponentStore.at(entityId);
    auto componentIter = componentMap.begin();
    while (componentIter != componentMap.end())
    {
        componentUsageMask |= mComponentMasks.at(componentIter->first);
        componentIter++;
    }

    return componentUsageMask;
}

void ecs::World::RemoveEntitiesWithoutAnyComponents()
{
    auto entityIter = mEntityComponentStore.begin();
    while (entityIter != mEntityComponentStore.end())
    {
        if (CalculateEntityComponentUsageMask(entityIter->first) == EMPTY_COMPONENT_MASK)
        {
            entityIter = mEntityComponentStore.erase(entityIter);
        }
        else
        {
            entityIter++;
        }
    }
}

void ecs::World::CongregateActiveEntitiesInCurrentFrame()
{
    mActiveEntitiesInFrame.clear();
    mActiveEntitiesInFrame.resize(mEntityComponentStore.size());

    for (const auto& entityEntry : mEntityComponentStore)
    {
        mActiveEntitiesInFrame.push_back(entityEntry.first);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::BaseSystem::BaseSystem(World& world)
: mWorld(world)
{
}

bool ecs::BaseSystem::ShouldProcessEntity(const EntityId entityId) const
{
    return (mWorld.CalculateEntityComponentUsageMask(entityId) & mComponentUsageMask) == mComponentUsageMask;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
