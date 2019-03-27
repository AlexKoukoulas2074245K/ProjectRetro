//
//  ECS.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 26/03/2019.
//

#include "ECS.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

bool BaseSystem::ShouldProcessEntity(const EntityId entityId, const World& world) const
{
    return (world.CalculateEntityComponentUsageMask(entityId) & mComponentUsageMask) == mComponentUsageMask;
}

const ComponentMask& BaseSystem::GetComponentUsageMask() const
{
    return mComponentUsageMask;    
}

void BaseSystem::SetComponentUsageMask(const ComponentMask& componentUsageMask)
{
    mComponentUsageMask = componentUsageMask;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const EntityComponentStoreMap& World::GetEntityComponentStore() const
{
    return mEntityComponentStore;
}

void World::Update(const float dt)
{
    RemoveEntitiesWithoutAnyComponents();
    for (auto& system : mSystems)
    {
        system->VUpdate(dt, *this);
    }
}

void World::RemoveEntity(const EntityId entityId)
{
    assert(mEntityComponentStore.count(entityId) != 0 &&
        "Entity does not exist in the world");

    mEntityComponentStore.at(entityId).clear();
}

ComponentMask World::CalculateEntityComponentUsageMask(const EntityId entityId) const
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

void World::RemoveEntitiesWithoutAnyComponents()
{
    auto entityIter = mEntityComponentStore.begin();
    while (entityIter != mEntityComponentStore.end())
    {
        if (CalculateEntityComponentUsageMask(entityIter->first) == 0)
        {
            entityIter = mEntityComponentStore.erase(entityIter);
        }
        else
        {
            entityIter++;
        }
    }
}