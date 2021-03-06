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

#include <typeinfo>

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
    RemoveMarkedSystems();
    RemoveEntitiesWithoutAnyComponents();
    CongregateActiveEntitiesInCurrentFrame();

    for(const auto& system: mSystems)
    {        
        system->VUpdateAssociatedComponents(dt);
        InsertNewEntitiesIntoActiveCollection();
    }
}

void ecs::World::DestroyEntity(const EntityId entityId)
{
    assert(entityId != NULL_ENTITY_ID &&
        "NULL_ENTITY_ID entity removal request");

    assert(mEntityComponentStore.count(entityId) != 0 &&
        "Entity does not exist in the world");

    auto& entityEntry = mEntityComponentStore.at(entityId);
    entityEntry.mComponentMap.clear();   
    entityEntry.mComponentMask.reset();
}

ecs::ComponentMask ecs::World::CalculateComponentUsageMaskForEntity(const EntityId entityId) const
{
    assert(entityId != NULL_ENTITY_ID &&
        "Mask calculation requested for NULL_ENTITY_ID");

    return mEntityComponentStore.at(entityId).mComponentMask;
}

void ecs::World::RemoveMarkedSystems()
{
    for (const auto& systemHash: mSystemHashesToRemove)
    {
        auto systemsIter = mSystems.begin();
        while (systemsIter != mSystems.end())
        {
            auto& system = *(*systemsIter);
            if (GetStringHash(typeid(system).name()) == systemHash)
            {
                systemsIter = mSystems.erase(systemsIter);
            }
            else
            {
                systemsIter++;
            }
        }
    }
}

void ecs::World::RemoveEntitiesWithoutAnyComponents()
{
    auto entityIter = mEntityComponentStore.begin();
    while (entityIter != mEntityComponentStore.end())
    {                
        if (entityIter->second.mComponentMap.size() == 0)
        {
            const auto id = entityIter->first;
            (void)id;
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
    mActiveEntitiesInFrame.reserve(mEntityComponentStore.size());

    for (const auto& entityEntry : mEntityComponentStore)
    {
        mActiveEntitiesInFrame.push_back(entityEntry.first);
    }
}

void ecs::World::InsertNewEntitiesIntoActiveCollection()
{
    if (mAddedEntitiesBySystemsUpdate.size() > 0)
    {
        mActiveEntitiesInFrame.insert(mActiveEntitiesInFrame.end(), mAddedEntitiesBySystemsUpdate.begin(), mAddedEntitiesBySystemsUpdate.end());
        mAddedEntitiesBySystemsUpdate.clear();
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
    assert(entityId != NULL_ENTITY_ID &&
        "Entity process check for NULL_ENTITY_ID");

    return (mWorld.CalculateComponentUsageMaskForEntity(entityId) & mComponentUsageMask) == mComponentUsageMask;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
