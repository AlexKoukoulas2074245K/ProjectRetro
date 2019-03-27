//
//  ECS.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 26/03/2019.
//

#ifndef ECS_h
#define ECS_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "util/TypeTraits.h"

#include <bitset>        // bitset
#include <memory>        // unique_ptr
#include <vector>        // vector
#include <unordered_map> // unordered_map
#include <cassert>       // assert

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

// Max component type count allowed
static constexpr int MAX_COMPONENTS       = 16;
static constexpr int EMPTY_COMPONENT_MASK = 0;

using ComponentMask   = std::bitset<MAX_COMPONENTS>;
using ComponentTypeId = int;
using EntityId        = long long;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct ComponentTypeIdHasher
{
    std::size_t operator()(const ComponentTypeId& key) const
    {
        return static_cast<std::size_t>(key);
    }
};

struct EntityIdHasher
{
    std::size_t operator()(const EntityId& key) const
    {
        return static_cast<std::size_t>(key);
    }
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class IComponent
{
public:
    virtual ~IComponent() = default;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class ISystem
{
public:
    virtual ~ISystem() = default;
    ISystem(const ISystem&) = delete;
    const ISystem& operator = (const ISystem&) = delete;
    
    inline const ComponentMask& GetComponentUsageMask() const { return mComponentUsageMask; }
    inline void SetComponentUsageMask(const ComponentMask& componentUsageMask) { mComponentUsageMask = componentUsageMask; }
    
    virtual void VUpdate(const float dt) = 0;
    
protected:
    ISystem() = default;
    
    ComponentMask mComponentUsageMask;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class World final
{
public:
    World()
    {
        mSystems.reserve(20);
        mEntityComponentStore.reserve(100);
    }
    
    inline EntityId CreateEntity()
    {
        return mEntityCounter++;
    }
    
    // Performs a single update of the world simulation
    inline void Update(const float dt)
    {
        RemoveDeadEntities();
        for (auto& system : mSystems)
        {
            system->VUpdate(dt);
        }
    }
    
    // Registers the given component type (ComponentType) to the world
    // and computes its mask
    template<class ComponentType>
    inline void RegisterComponentType()
    {
        static_assert(std::is_base_of<IComponent, ComponentType>::value,
                      "Attempted to register class not derived from IComponent");
        assert(mComponentMasks.size() != MAX_COMPONENTS);
        
        const auto componentTypeId = GetTypeHash<ComponentType>();
        mComponentMasks[componentTypeId] = 1 << mComponentMasks.size();
    }
    
    // Registers and takes ownership of the given system. Needs to be called
    // AFTER all component types have been registered to the world, as the
    // system mask (which is dependent on the registered components also gets computed here
    template<class FirstUtilizedComponentType, class... OtherUtilizedComponentTypes>
    inline void RegisterSystem(std::unique_ptr<ISystem> system)
    {
        assert(mComponentMasks.size() != 0);
        system->SetComponentUsageMask(CalculateComponentUsageMask<FirstUtilizedComponentType, OtherUtilizedComponentTypes...>());
        mSystems.push_back(std::move(system));
    }
    
    inline ComponentMask CalculateEntityComponentUsageMask(const EntityId entityId) const
    {
        ComponentMask componentUsageMask;
        const auto& componentMap = mEntityComponentStore.at(entityId);
        auto componentIter = componentMap.begin();
        while (componentIter != componentMap.end())
        {
            componentUsageMask |= mComponentMasks.at(componentIter->first);
        }
        
        return componentUsageMask;
    }

private:
    // Calculates at compile time the bit mask of the given template argument (FirstUtilizedComponentType)
    template<class FirstUtilizedComponentType>
    inline ComponentMask CalculateComponentUsageMask() const
    {
        static_assert(std::is_base_of<IComponent, FirstUtilizedComponentType>::value,
                      "Attempted to extract mask from class not derived from IComponent");
        return mComponentMasks.at(GetTypeHash<FirstUtilizedComponentType>());
    }
    
    // Recursively calculates at compile time the bit mask formed of all given templates arguments
    template<class FirstUtilizedComponentType, class SecondUtilizedComponentType, class ...RestUtilizedComponentTypes>
    inline ComponentMask CalculateComponentUsageMask() const
    {
        static_assert(std::is_base_of<IComponent, FirstUtilizedComponentType>::value,
                      "Attempted to extract mask from class not derived from IComponent");
        return mComponentMasks.at(GetTypeHash<FirstUtilizedComponentType>()) |
            CalculateComponentUsageMask<SecondUtilizedComponentType, RestUtilizedComponentTypes...>();
    }
    
    inline void RemoveDeadEntities()
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
    
private:
    using ComponentMap            = std::unordered_map<ComponentTypeId, std::unique_ptr<IComponent>, ComponentTypeIdHasher>;
    using EntityComponentStoreMap = std::unordered_map<EntityId, ComponentMap, EntityIdHasher>;
    using ComponentMaskMap        = std::unordered_map<ComponentTypeId, ComponentMask, ComponentTypeIdHasher>;
    
    EntityComponentStoreMap mEntityComponentStore;
    ComponentMaskMap        mComponentMasks;
    
    std::vector<std::unique_ptr<ISystem>> mSystems;
    
    EntityId mEntityCounter = 0;
};

#endif /* ECS_h */
