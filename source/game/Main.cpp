//
//  Main.cpp
//  Hardcore2D
//
//  Created by Alex Koukoulas on 05/01/2019.
//

#include "ECS.h"
#include "util/MathUtils.h"

#include <iostream>
#include <chrono>

const int ENTITY_COUNT = 10000;

class DummyBooleanComponent : public IComponent
{
public:
    DummyBooleanComponent(const bool val) : mBoolean(val) {}
    bool mBoolean;
};

class DummyPositionComponent : public IComponent
{
public:
    DummyPositionComponent(const float x, const float y) : mX(x), mY(y) {}
    float mX = 0.0f;
    float mY = 0.0f;
};

class DummyDimensionsComponent : public IComponent
{
public:
    DummyDimensionsComponent(const float width, const float height) : mWidth(width), mHeight(height) {}
    float mWidth = 1.0f;
    float mHeight = 1.0f;
};


class IntersectionSystem: public BaseSystem
{
public:
    void VUpdate(const float, World& world) override 
    {
        const auto& entityComponentStore = world.GetEntityComponentStore();
        for (const auto& entityEntry : entityComponentStore)
        {
            const auto entityId = entityEntry.first;
            if (world.HasComponent<DummyPositionComponent>(entityId) && world.HasComponent<DummyDimensionsComponent>(entityId))
            {
                const auto& positionComponent = world.GetComponent<DummyPositionComponent>(entityId);
                const auto& dimensionsComponent = world.GetComponent<DummyDimensionsComponent>(entityId);

                mIntersecting = positionComponent.mX == positionComponent.mY + logf(dimensionsComponent.mWidth) + sqrtf(positionComponent.mY);
            }

            world.RemoveEntity(RandomInt(0, 199));
        }
    }

private:
    bool mIntersecting = false;
};


int main(int, char**)
{
    World world;
    world.RegisterComponentType<DummyBooleanComponent>();
    world.RegisterComponentType<DummyPositionComponent>();
    world.RegisterComponentType<DummyDimensionsComponent>();
    
    world.RegisterSystem<DummyPositionComponent, DummyDimensionsComponent>(std::make_unique<IntersectionSystem>());

    for (int i = 0; i < ENTITY_COUNT; ++i)
    {
        const auto generatedEntityId = world.CreateEntity();

        const auto randomChoice = RandomInt(0, 2);
        if (randomChoice == 0)
        {
            world.AddComponent<DummyDimensionsComponent>(generatedEntityId, std::make_unique<DummyDimensionsComponent>(RandomFloat(), RandomFloat()));
            world.AddComponent<DummyPositionComponent>(generatedEntityId, std::make_unique<DummyPositionComponent>(RandomFloat(), RandomFloat()));
        }
        else if (randomChoice == 1)
        {
            world.AddComponent<DummyDimensionsComponent>(generatedEntityId, std::make_unique<DummyBooleanComponent>(true));
        }
        else if (randomChoice == 2)
        {
            world.AddComponent<DummyPositionComponent>(generatedEntityId, std::make_unique<DummyPositionComponent>(RandomFloat(), RandomFloat()));
        }
    }

    std::cout << "Profiling for TypeIndex starting: " << std::endl;
    float miliSecondAccumulator = 0.0f;
    for (int i = 0; i < 10; ++i)
    {
        auto start = std::chrono::high_resolution_clock::now();
        world.Update(1.0f);        
        auto finish = std::chrono::high_resolution_clock::now();
        std::cout << "Iteration " << i << " took: " << (std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count()) / 1000000.0f << "ms\n";
        miliSecondAccumulator += (std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count()) / 1000000.0f;
    }
    std::cout << "Average update: " << miliSecondAccumulator / 10.0f << "ms" << std::endl;
    std::cout << "Profiling for TypeIndex ended: " << std::endl;

    std::getchar();
}

