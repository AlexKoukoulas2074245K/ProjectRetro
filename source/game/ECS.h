//
//  ECS.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 26/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef ECS_h
#define ECS_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "common_utils/TypeTraits.h"

#include <bitset>        
#include <cassert>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>        

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

namespace ecs
{

// Max component type count allowed
static constexpr int MAX_COMPONENTS       = 16;
static constexpr int EMPTY_COMPONENT_MASK = 0;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class World;
class BaseSystem;
class IComponent;

using ComponentMask   = std::bitset<MAX_COMPONENTS>;
using ComponentTypeId = int;
using SystemTypeId    = int;
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

struct SystemTypeIdHasher
{
    std::size_t operator()(const SystemTypeId& key) const
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

class BaseSystem
{
public:
    BaseSystem(World&);

    virtual ~BaseSystem() = default;
    BaseSystem(const BaseSystem&) = delete;
    const BaseSystem& operator = (const BaseSystem&) = delete;
    
    virtual void VUpdate(const float dt) = 0;
    
protected:          
    // Determines whether the given entity (entityId) should be processed by this system
    // based on their respective component usage masks
    bool ShouldProcessEntity(EntityId) const;

    World& mWorld;

protected:
    ComponentMask mComponentUsageMask;    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class World final
{
public:
    World();
    
    // Returns a non-mutable reference to the active entity component store
    const std::vector<EntityId>& GetActiveEntities() const;

    // Performs a single update of the world simulation
    void Update(const float dt);

    // Removes the given entity from the world.    
    // Internally simply clears the component store for the given entity since all entities 
    // with no attached components will be cleaned up at the begining of the next frame    
    void RemoveEntity(const EntityId);
    
    // Calculates the aggregate component mask for a given entity id (entityId)
    ComponentMask CalculateEntityComponentUsageMask(const EntityId entityId) const;

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

        const auto componentTypeId = GetTypeHash<ComponentType>();
        assert(mEntityComponentStore.at(entityId).count(componentTypeId) != 0 &&
            "Component is not present in this entity's component store");
        
        return static_cast<ComponentType&>(*mEntityComponentStore.at(entityId).at(componentTypeId)); 
    }

    // Checks whether the given entity (entityId) has a component of type ComponentType
    template<class ComponentType>
    inline bool HasComponent(const EntityId entityId) const
    {
        assert(mEntityComponentStore.count(entityId) != 0 &&
            "Entity does not exist in the world");

        const auto componentTypeId = GetTypeHash<ComponentType>();
        return mEntityComponentStore.at(entityId).count(componentTypeId) != 0;
    }

    // Adds the given component (component) to the component map of the given entity (entityId)
    template<class ComponentType>
    inline void AddComponent(const EntityId entityId, std::unique_ptr<IComponent> component)
    {
        assert(mEntityComponentStore.count(entityId) != 0 &&
            "Entity does not exist in the world");

        const auto componentTypeId = GetTypeHash<ComponentType>();
        assert(mEntityComponentStore.at(entityId).count(componentTypeId) == 0 &&
            "Component is already present in this entity's component store");

        mEntityComponentStore.at(entityId)[componentTypeId] = std::move(component);
    }

    // Removes the component of type ComponentType from the component map of the given entity (entityid)
    template<class ComponentType>
    inline void RemoveComponent(const EntityId entityId)
    {
        assert(mEntityComponentStore.count(entityId) != 0 &&
            "Entity does not exist in the world");

        const auto componentTypeId = GetTypeHash<ComponentType>();
        assert(mEntityComponentStore.at(entityId).count(componentTypeId) != 0 &&
            "Component is not present in this entity's component store");
        
        mEntityComponentStore.at(entityId).erase(componentTypeId);
    }

    // Registers the given component type (ComponentType) to the world
    // and computes its mask. All components that are used as data containers 
    // for entities, must be registered here first before used
    template<class ComponentType>
    inline void RegisterComponentType()
    {
        static_assert(std::is_base_of<IComponent, ComponentType>::value,
            "ComponentType does not derive from IComponent");
        assert(mComponentMasks.size() != MAX_COMPONENTS &&
            "Exceeded maximum number of different component types");
        
        const auto componentTypeId = GetTypeHash<ComponentType>();
        mComponentMasks[componentTypeId] = 1LL << mComponentMasks.size();
    }
    
    template<class ComponentType>
    inline ComponentType& GetSingletonComponent()
    {
        static_assert(std::is_base_of<IComponent, ComponentType>::value,
            "ComponentType does not derive from IComponent");

        const auto componentTypeId = GetTypeHash<ComponentType>();
        return static_cast<ComponentType&>(*mSingletonComponents.at(componentTypeId));
    }

    // Takes ownership of the given component and makes it as a singleton component of its type in the world.
    // There can only be one singleton component of each singleton component type
    // and is accessed via the GetSingletonComponent method
    template<class ComponentType>
    inline void SetSingletonComponent(std::unique_ptr<IComponent> component)
    {
        static_assert(std::is_base_of<IComponent, ComponentType>::value,
            "ComponentType does not derive from IComponent");

        const auto componentTypeId = GetTypeHash<ComponentType>();
        assert(mSingletonComponents.count(componentTypeId) == 0 &&
            "A Singleton component of the specified type already exists in the world");

        mSingletonComponents[componentTypeId] = std::move(component);
    }

    // Removes the current singleton component of the specified type (ComponentType)
    // from the world
    template<class ComponentType>
    inline void RemoveSingletonComponent()
    {
        static_assert(std::is_base_of<IComponent, ComponentType>::value,
            "ComponentType does not derive from IComponent");
                
        const auto componentTypeId = GetTypeHash<ComponentType>();
        assert(mSingletonComponents.count(componentTypeId) != 0 &&
            "A Singleton component of the specified type does not exist");

        mSingletonComponents.erase(componentTypeId);
    }

    // Sets and takes ownership of the given system.
    template<class SystemType>
    inline void SetSystem(std::unique_ptr<BaseSystem> system)
    {
        const auto systemTypeId = GetTypeHash<SystemType>();
        assert(mSystems.count(systemTypeId) == 0 &&
            "System of the same type already registered in the world");
        
        mSystems[systemTypeId] = std::move(system);
    }            

    // Removes the system of the specified type registered in the world
    template<class SystemType>
    inline void RemoveSystem()
    {
        static_assert(std::is_base_of<BaseSystem, SystemType>::value,
            "SystemType does not derive from BaseSystem");
        
        const auto systemTypeId = GetTypeHash<SystemType>();
        assert(mSystems.count(systemTypeId) != 0 &&
            "System of the specified type is not registered in the world");

        mSystems.erase(systemTypeId);
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
    // Removes all entities with no components currently attached to them
    void RemoveEntitiesWithoutAnyComponents();

    // Collects all active entities (with at least one component) for processing by systems for this frame
    void CongregateActiveEntitiesInCurrentFrame();

    // Initializes a component of the given type for the given entity (entityId)
    template<class FirstUtilizedComponentType>
    inline void InitializeEmptyComponentForEntity(const EntityId entityId)
    {
        const auto componentTypeId = GetTypeHash<FirstUtilizedComponentType>();
        assert(mEntityComponentStore[entityId].count(componentTypeId) == 0 &&
            "A component of that type already exists in this entity's component store");
        mEntityComponentStore[entityId][componentTypeId] = std::make_unique<FirstUtilizedComponentType>();
    }

    // Recusrively initializes various different components of the given types for the given entity (entityId)
    template<class FirstUtilizedComponentType, class SecondUtilizedComponentType, class ...RestUtilizedComponentTypes>
    inline void InitializeEmptyComponentForEntity(const EntityId entityId)
    {
        InitializeEmptyComponentForEntity<FirstUtilizedComponentType>(entityId);
        InitializeEmptyComponentForEntity<SecondUtilizedComponentType, RestUtilizedComponentTypes...>(entityId);
    }   
    
private:    
    using SystemsMap              = std::map<SystemTypeId, std::unique_ptr<BaseSystem>>;
    using ComponentMap            = std::unordered_map<ComponentTypeId, std::unique_ptr<IComponent>, ComponentTypeIdHasher>;
    using EntityComponentStoreMap = std::unordered_map<EntityId, ComponentMap, EntityIdHasher>;
    using ComponentMaskMap        = std::unordered_map<ComponentTypeId, ComponentMask, ComponentTypeIdHasher>;

    EntityComponentStoreMap mEntityComponentStore;
    ComponentMaskMap        mComponentMasks;    
    ComponentMap            mSingletonComponents;
    SystemsMap              mSystems;
    
    std::vector<EntityId> mActiveEntitiesInFrame;

    EntityId mEntityCounter = 0LL;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

}

#endif /* ECS_h */
