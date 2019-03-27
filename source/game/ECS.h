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
#include <forward_list>  // forward_list

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

// Max component type count allowed
static constexpr int MAX_COMPONENTS       = 16;
static constexpr int EMPTY_COMPONENT_MASK = 0;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class World;
class BaseSystem;
class IComponent;

using ComponentMask           = std::bitset<MAX_COMPONENTS>;
using ComponentTypeId         = int;
using EntityId                = long long;

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

using ComponentMap            = std::unordered_map<ComponentTypeId, std::unique_ptr<IComponent>, ComponentTypeIdHasher>;
using EntityComponentStoreMap = std::unordered_map<EntityId, ComponentMap, EntityIdHasher>;
using ComponentMaskMap        = std::unordered_map<ComponentTypeId, ComponentMask, ComponentTypeIdHasher>;

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

class BaseSystem
{
public:
    virtual ~BaseSystem() = default;
    BaseSystem(const BaseSystem&) = delete;
    const BaseSystem& operator = (const BaseSystem&) = delete;
    
    // Determines whether the given entity (entityId) should be processed by this system
    // based on their respective component usage masks
    bool ShouldProcessEntity(EntityId, const World&) const;

    // Gets this system's component usage mask
    const ComponentMask& GetComponentUsageMask() const;

    // Sets this system's component usage mask
    void SetComponentUsageMask(const ComponentMask&);
    
    virtual void VUpdate(const float dt, World&) = 0;
    
protected:
    BaseSystem() = default;        

protected:
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
    
    // Returns a non-mutable reference to the active entity component store
    const EntityComponentStoreMap& GetEntityComponentStore() const;

    // Performs a single update of the world simulation
    void Update(const float dt);

    // Removes the given entity from the world.    
    // Internally simply clears the component store for the given entity since all entities 
    // with no attached components will be cleaned up at the begining of the next frame    
    void RemoveEntity(const EntityId);
    
    // Creates an entity and returns its corresponding entity id. 
    // Also initializes a components of the specified type to be used by the entity
    template<class FirstUtilizedComponentType>
    inline EntityId CreateEntity()
    {
        InitializeEmptyComponentForEntity<FirstUtilizedComponentType>(mEntityCounter);
        return mEntityCounter++;
    }

    // Creates an entity and returns its corresponding entity id. 
    // Also initializes a list of components of the specified types to be used by the entity
    template<class FirstUtilizedComponentType, class SecondUtilizedComponentType, class ...RestUtilizedComponentTypes>
    inline EntityId CreateEntity()
    {                
        InitializeEmptyComponentForEntity<FirstUtilizedComponentType, SecondUtilizedComponentType, RestUtilizedComponentTypes...>(mEntityCounter);
        return mEntityCounter++;
    }

    // Creates an entity and returns its corresponding entity id. 
    inline EntityId CreateEntity()
    {
        mEntityComponentStore[mEntityCounter];
        return mEntityCounter++;
    }
    
    // Gets the attached component (of type ComponentType) to the given entity (entityId)
    template<class ComponentType>
    inline ComponentType& GetComponent(const EntityId entityId) const
    {        
        assert(mEntityComponentStore.count(entityId) != 0 &&
            "Entity does not exist in the world");

        const auto componentTypeHash = GetTypeHash<ComponentType>();
        assert(mEntityComponentStore.at(entityId).count(componentTypeHash) != 0 &&
            "Component is not present in this entity's component store");
        
        return static_cast<ComponentType&>(*mEntityComponentStore.at(entityId).at(componentTypeHash)); 
    }

    // Checks whether the given entity (entityId) has a component of type ComponentType
    template<class ComponentType>
    inline bool HasComponent(const EntityId entityId) const
    {
        assert(mEntityComponentStore.count(entityId) != 0 &&
            "Entity does not exist in the world");

        const auto componentTypeHash = GetTypeHash<ComponentType>();
        return mEntityComponentStore.at(entityId).count(componentTypeHash) != 0;
    }

    // Adds the given component (component) to the component map of the given entity (entityId)
    template<class ComponentType>
    inline void AddComponent(const EntityId entityId, std::unique_ptr<IComponent> component)
    {
        assert(mEntityComponentStore.count(entityId) != 0 &&
            "Entity does not exist in the world");

        const auto componentTypeHash = GetTypeHash<ComponentType>();
        assert(mEntityComponentStore.at(entityId).count(componentTypeHash) == 0 &&
            "Component is already present in this entity's component store");

        mEntityComponentStore.at(entityId)[componentTypeHash] = std::move(component);
    }

    // Removes the component of type ComponentType from the component map of the given entity (entityid)
    template<class ComponentType>
    inline void RemoveComponent(const EntityId entityId)
    {
        assert(mEntityComponentStore.count(entityId) != 0 &&
            "Entity does not exist in the world");

        const auto componentTypeHash = GetTypeHash<ComponentType>();
        assert(mEntityComponentStore.at(entityId).count(componentTypeHash) != 0 &&
            "Component is not present in this entity's component store");
        
        mEntityComponentStore.at(entityId).erase(componentTypeHash);
    }

    // Registers the given component type (ComponentType) to the world
    // and computes its mask
    template<class ComponentType>
    inline void RegisterComponentType()
    {
        static_assert(std::is_base_of<IComponent, ComponentType>::value,
            "Attempted to register class not derived from IComponent");
        assert(mComponentMasks.size() != MAX_COMPONENTS &&
            "Exceeded maximum number of different component types");
        
        const auto componentTypeId = GetTypeHash<ComponentType>();
        mComponentMasks[componentTypeId] = 1LL << mComponentMasks.size();
    }
    
    // Registers and takes ownership of the given system. Needs to be called
    // AFTER all component types have been registered to the world, as the
    // system mask (which is dependent on the registered components also gets computed here
    template<class FirstUtilizedComponentType, class... OtherUtilizedComponentTypes>
    inline void RegisterSystem(std::unique_ptr<BaseSystem> system)
    {
        assert(mComponentMasks.size() != 0 &&
            "No masks have been registered in the world for any component");
        system->SetComponentUsageMask(CalculateComponentUsageMask<FirstUtilizedComponentType, OtherUtilizedComponentTypes...>());
        mSystems.push_back(std::move(system));
    }
    
    // Calculates the aggregate component mask for a given entity id (entityId)
    ComponentMask CalculateEntityComponentUsageMask(const EntityId entityId) const;

private:    
    void RemoveEntitiesWithoutAnyComponents();

    // Initializes a component of the given type for the given entity (entityId)
    template<class FirstUtilizedComponentType>
    inline void InitializeEmptyComponentForEntity(const EntityId entityId)
    {
        const auto componentTypeHash = GetTypeHash<FirstUtilizedComponentType>();
        assert(mEntityComponentStore[entityId].count(componentTypeHash) == 0 &&
            "A component of that type already exists in this entity's component store");
        mEntityComponentStore[entityId][componentTypeHash] = std::make_unique<FirstUtilizedComponentType>();
    }

    // Recusrively initializes various different components of the given types for the given entity (entityId)
    template<class FirstUtilizedComponentType, class SecondUtilizedComponentType, class ...RestUtilizedComponentTypes>
    inline void InitializeEmptyComponentForEntity(const EntityId entityId)
    {
        InitializeEmptyComponentForEntity<FirstUtilizedComponentType>(entityId);
        InitializeEmptyComponentForEntity<SecondUtilizedComponentType, RestUtilizedComponentTypes...>(entityId);
    }

    // Calculates the bit mask of the given template argument (FirstUtilizedComponentType)
    template<class FirstUtilizedComponentType>
    inline ComponentMask CalculateComponentUsageMask() const
    {
        static_assert(std::is_base_of<IComponent, FirstUtilizedComponentType>::value,
                      "Attempted to extract mask from class not derived from IComponent");
        assert(mComponentMasks.count(GetTypeHash<FirstUtilizedComponentType>()) != 0 && 
                      "Component type not registered in the world");

        return mComponentMasks.at(GetTypeHash<FirstUtilizedComponentType>());
    }
    
    // Recursively calculates the bit mask formed of all given templates arguments
    template<class FirstUtilizedComponentType, class SecondUtilizedComponentType, class ...RestUtilizedComponentTypes>
    inline ComponentMask CalculateComponentUsageMask() const
    {
        static_assert(std::is_base_of<IComponent, FirstUtilizedComponentType>::value,
                      "Attempted to extract mask from class not derived from IComponent");
        assert(mComponentMasks.count(GetTypeHash<FirstUtilizedComponentType>()) != 0 && 
                      "Component type not registered in the world");

        return mComponentMasks.at(GetTypeHash<FirstUtilizedComponentType>()) |
            CalculateComponentUsageMask<SecondUtilizedComponentType, RestUtilizedComponentTypes...>();
    }        
    
private:    
    EntityComponentStoreMap mEntityComponentStore;
    ComponentMaskMap        mComponentMasks;
    
    std::vector<std::unique_ptr<BaseSystem>> mSystems;
    
    EntityId mEntityCounter = 0LL;
};

#endif /* ECS_h */
