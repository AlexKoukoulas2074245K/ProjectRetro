//
//  App.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "App.h"
#include "common_components/TransformComponent.h"
#include "rendering/components/RenderableComponent.h"
#include "rendering/components/WindowComponent.h"
#include "rendering/systems/RenderingSystem.h"
#include "resources/ResourceLoadingService.h"

#include <SDL_events.h> 
#include <SDL_timer.h>  

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void App::Run()
{
    RegisterCommonComponents();
    InitializeSystems();
    GameLoop();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void App::RegisterCommonComponents()
{
    mWorld.RegisterComponentType<RenderableComponent>();
    mWorld.RegisterComponentType<TransformComponent>();
}

void App::InitializeSystems()
{
    mWorld.SetSystem<RenderingSystem>(std::make_unique<RenderingSystem>(mWorld));
}

void App::GameLoop()
{    
    float elapsedTicks          = 0.0f;
    float dtAccumulator         = 0.0f;
    long long framesAccumulator = 0;
    
    const auto dummyEntity = mWorld.CreateEntity();
    
    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition.x = -2.0f;
    
    auto renderableComponent = std::make_unique<RenderableComponent>();
    renderableComponent->mShaderNameId = StringId("basic");
    renderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource("textures/materials/overworld.png");
    renderableComponent->mMeshResourceId = ResourceLoadingService::GetInstance().LoadResource("models/camera_facing_quad[0.250000,1.000000-0.125000,1.000000-0.125000,0.984375-0.250000,0.984375].obj");
    
    mWorld.AddComponent<TransformComponent>(dummyEntity, std::move(transformComponent));
    mWorld.AddComponent<RenderableComponent>(dummyEntity, std::move(renderableComponent));
    
    const auto dummyEntity2 = mWorld.CreateEntity();
    
    auto transformComponent2 = std::make_unique<TransformComponent>();
    transformComponent2->mPosition.x = 2.0f;
    
    auto renderableComponent2 = std::make_unique<RenderableComponent>();
    renderableComponent2->mShaderNameId = StringId("basic");
    renderableComponent2->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource("textures/materials/overworld.png");
    renderableComponent2->mMeshResourceId = ResourceLoadingService::GetInstance().LoadResource("models/camera_facing_quad[0.125000,1.000000-0.000000,1.000000-0.000000,0.984375-0.125000,0.984375].obj");
    
    mWorld.AddComponent<TransformComponent>(dummyEntity2, std::move(transformComponent2));
    mWorld.AddComponent<RenderableComponent>(dummyEntity2, std::move(renderableComponent2));
    
    while (!AppShouldQuit())
    {
        // Calculate frame delta
        const auto currentTicks = static_cast<float>(SDL_GetTicks());
        auto lastFrameTicks     = currentTicks - elapsedTicks;
        elapsedTicks            = currentTicks;
        const auto dt           = lastFrameTicks * 0.001f;

        framesAccumulator++;
        dtAccumulator += dt;
        
        auto& transformComponent1 = mWorld.GetComponent<TransformComponent>(dummyEntity);
        auto& transformComponent2 = mWorld.GetComponent<TransformComponent>(dummyEntity2);
        
        transformComponent1.mPosition.z -= 0.1f * dt;
        transformComponent2.mPosition.z -= 0.1f * dt;
#ifndef NDEBUG
        if (dtAccumulator > 1.0f)
        {
            const auto windowTitle = "FPS: " + std::to_string(framesAccumulator);
            const auto& windowComponent = mWorld.GetSingletonComponent<WindowComponent>();
            
            SDL_SetWindowTitle(windowComponent.mWindowHandle, windowTitle.c_str());
            
            framesAccumulator = 0;
            dtAccumulator     = 0.0f;
        }
#endif        

        // Simulate world
        mWorld.Update(dt);
    }
}

bool App::AppShouldQuit()
{
    // Poll events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT: return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
