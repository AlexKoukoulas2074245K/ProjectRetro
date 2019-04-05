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
#include "common/GameConstants.h"
#include "common/components/TransformComponent.h"
#include "common/components/PlayerTagComponent.h"
#include "input/components/InputStateComponent.h"
#include "input/systems/RawInputHandlingSystem.h"
#include "rendering/components/AnimationTimerComponent.h"
#include "rendering/components/RenderableComponent.h"
#include "rendering/components/WindowComponent.h"
#include "rendering/systems/AnimationSystem.h"
#include "rendering/systems/CameraControlSystem.h"
#include "rendering/systems/RenderingSystem.h"
#include "resources/ResourceLoadingService.h"
#include "overworld/components/LevelGridComponent.h"
#include "overworld/utils/LevelUtils.h"

#include <SDL_events.h> 
#include <SDL_timer.h>  

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void App::Run()
{    
    InitializeSystems();
    GameLoop();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void App::InitializeSystems()
{
    mWorld.SetSystem<RawInputHandlingSystem>(std::make_unique<RawInputHandlingSystem>(mWorld));
    mWorld.SetSystem<AnimationSystem>(std::make_unique<AnimationSystem>(mWorld));
    mWorld.SetSystem<CameraControlSystem>(std::make_unique<CameraControlSystem>(mWorld));
    mWorld.SetSystem<RenderingSystem>(std::make_unique<RenderingSystem>(mWorld));
}

void App::GameLoop()
{    
    float elapsedTicks          = 0.0f;
    float dtAccumulator         = 0.0f;
    long long framesAccumulator = 0;
    
    const auto levelEntity = mWorld.CreateEntity();
    const auto otherDummyEntity = mWorld.CreateEntity();
    const auto dummyEntity = mWorld.CreateEntity();
    const auto dummyEntity2 = mWorld.CreateEntity();
    
    auto levelGridComponent = std::make_unique<LevelGridComponent>();
    levelGridComponent->mLevelGrid = InitializeLevelGridOfDimensions(5, 5);
    mWorld.AddComponent<LevelGridComponent>(levelEntity, std::move(levelGridComponent));

    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition.x = -2.0f;
    transformComponent->mPosition.z = -2.0f;
    
    auto renderableComponent = std::make_unique<RenderableComponent>();
    renderableComponent->mShaderNameId = StringId("basic");
    renderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource("textures/materials/overworld.png");
    renderableComponent->mAnimationsToMeshes[StringId("test")].push_back(ResourceLoadingService::GetInstance().LoadResource("models/camera_facing_quad[0.250000,1.000000-0.125000,1.000000-0.125000,0.984375-0.250000,0.984375].obj"));
    renderableComponent->mActiveAnimationNameId = StringId("test");

    mWorld.AddComponent<TransformComponent>(dummyEntity, std::move(transformComponent));
    mWorld.AddComponent<RenderableComponent>(dummyEntity, std::move(renderableComponent));
    
    auto othertransformComponent = std::make_unique<TransformComponent>();
    othertransformComponent->mPosition.x = -1.5f;    

    auto otherrenderableComponent = std::make_unique<RenderableComponent>();
    otherrenderableComponent->mShaderNameId = StringId("basic");
    otherrenderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource("textures/materials/overworld.png");
    otherrenderableComponent->mAnimationsToMeshes[StringId("test")].push_back(ResourceLoadingService::GetInstance().LoadResource("models/camera_facing_quad[0.250000,1.000000-0.125000,1.000000-0.125000,0.984375-0.250000,0.984375].obj"));
    otherrenderableComponent->mActiveAnimationNameId = StringId("test");
    
    mWorld.AddComponent<TransformComponent>(otherDummyEntity, std::move(othertransformComponent));
    mWorld.AddComponent<RenderableComponent>(otherDummyEntity, std::move(otherrenderableComponent));
    
    auto transformComponent2 = std::make_unique<TransformComponent>();
    transformComponent2->mPosition.x = 2.0f;
    transformComponent2->mPosition.x = -1.0f;
    
    auto renderableComponent2 = std::make_unique<RenderableComponent>();
    renderableComponent2->mShaderNameId = StringId("basic");
    renderableComponent2->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource("textures/materials/overworld.png");
    renderableComponent2->mAnimationsToMeshes[StringId("test")].push_back(ResourceLoadingService::GetInstance().LoadResource("models/camera_facing_quad[0.125000,1.000000-0.000000,1.000000-0.000000,0.984375-0.125000,0.984375].obj"));
    renderableComponent2->mAnimationsToMeshes[StringId("test")].push_back(ResourceLoadingService::GetInstance().LoadResource("models/camera_facing_quad[0.500000,1.000000-0.3750000,1.000000-0.375000,0.984375-0.500000,0.984375].obj"));
    renderableComponent2->mActiveAnimationNameId = StringId("test");

    auto animationComponent = std::make_unique<AnimationTimerComponent>();
    animationComponent->mAnimationTimer = std::make_unique<Timer>(0.5f);

    mWorld.AddComponent<AnimationTimerComponent>(dummyEntity2, std::move(animationComponent));
    mWorld.AddComponent<PlayerTagComponent>(dummyEntity2, std::make_unique<PlayerTagComponent>());
    mWorld.AddComponent<RenderableComponent>(dummyEntity2, std::move(renderableComponent2));
    mWorld.AddComponent<TransformComponent>(dummyEntity2, std::move(transformComponent2));
    
    bool topRightTexture = true;
    for (int y = 0; y < 5; ++y)
    {
        for (int x = 0; x < 5; ++x)
        {
            auto tileEntity = mWorld.CreateEntity();
            auto tileTransformComponent = std::make_unique<TransformComponent>();
            tileTransformComponent->mPosition = glm::vec3(x * OVERWORLD_TILE_SIZE, 0.0f, y * OVERWORLD_TILE_SIZE);

            auto tileRenderableComponent = std::make_unique<RenderableComponent>();
            tileRenderableComponent->mShaderNameId = StringId("basic");
            tileRenderableComponent->mActiveAnimationNameId = StringId("test");
            tileRenderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource(topRightTexture ? "textures/materials/2d_out_full_floor.png": "textures/materials/2d_out_empty_floor.png");
            tileRenderableComponent->mAnimationsToMeshes[StringId("test")].push_back(ResourceLoadingService::GetInstance().LoadResource("models/2d_out_empty_floor.obj"));

            mWorld.AddComponent<TransformComponent>(tileEntity, std::move(tileTransformComponent));
            mWorld.AddComponent<RenderableComponent>(tileEntity, std::move(tileRenderableComponent));

            topRightTexture = !topRightTexture;
        }
    }

    while (!AppShouldQuit())
    {
        // Calculate frame delta
        const auto currentTicks = static_cast<float>(SDL_GetTicks());
        auto lastFrameTicks     = currentTicks - elapsedTicks;
        elapsedTicks            = currentTicks;
        const auto dt           = lastFrameTicks * 0.001f;

        framesAccumulator++;
        dtAccumulator += dt;
        
        const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateComponent>();
        auto& transformComponent22 = mWorld.GetComponent<TransformComponent>(dummyEntity2);
               
        if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT) == VirtualActionInputState::PRESSED)
        {
            transformComponent22.mPosition.x -= 2.0f * dt;
        }
        else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT) == VirtualActionInputState::PRESSED)
        {
            transformComponent22.mPosition.x += 2.0f * dt;
        }
        else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP) == VirtualActionInputState::PRESSED)
        {
            transformComponent22.mPosition.z += 2.0f * dt;
        }
        else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN) == VirtualActionInputState::PRESSED)
        {
            transformComponent22.mPosition.z -= 2.0f * dt;
        }

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
