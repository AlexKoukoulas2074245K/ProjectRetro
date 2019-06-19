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
#include "common/components/DirectionComponent.h"
#include "common/components/TextboxComponent.h"
#include "common/components/TransformComponent.h"
#include "common/components/PlayerTagComponent.h"
#include "common/systems/GuiManagementSystem.h"
#include "input/components/InputStateSingletonComponent.h"
#include "input/systems/RawInputHandlingSystem.h"
#include "rendering/components/AnimationTimerComponent.h"
#include "rendering/components/RenderableComponent.h"
#include "rendering/components/RenderingContextSingletonComponent.h"
#include "rendering/components/WindowSingletonComponent.h"
#include "rendering/systems/AnimationSystem.h"
#include "rendering/systems/CameraControlSystem.h"
#include "rendering/systems/RenderingSystem.h"
#include "resources/MeshUtils.h"
#include "overworld/components/ActiveLevelSingletonComponent.h"
#include "overworld/components/LevelResidentComponent.h"
#include "overworld/components/LevelModelComponent.h"
#include "overworld/components/MovementStateComponent.h"
#include "overworld/systems/AnimatedFlowersAnimationSystem.h"
#include "overworld/systems/MovementControllerSystem.h"
#include "overworld/systems/NpcAiSystem.h"
#include "overworld/systems/PlayerActionControllerSystem.h"
#include "overworld/systems/SeaTilesAnimationSystem.h"
#include "overworld/systems/TransitionAnimationSystem.h"
#include "overworld/systems/WarpConnectionsSystem.h"
#include "overworld/utils/LevelUtils.h"
#include "overworld/utils/LevelLoadingUtils.h"
#include "overworld/utils/OverworldCharacterLoadingUtils.h"

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
    auto renderingSystem = std::make_unique<RenderingSystem>(mWorld);

    mWorld.AddSystem(std::make_unique<RawInputHandlingSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<GuiManagementSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<PlayerActionControllerSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<NpcAiSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<AnimationSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<MovementControllerSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<AnimatedFlowersAnimationSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<SeaTilesAnimationSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<TransitionAnimationSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<WarpConnectionsSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<CameraControlSystem>(mWorld));
    mWorld.AddSystem(std::move(renderingSystem));
}

void App::GameLoop()
{    
    float elapsedTicks          = 0.0f;
    float dtAccumulator         = 0.0f;
    long long framesAccumulator = 0;
    
    DummyInitialization();

    const auto& windowComponent           = mWorld.GetSingletonComponent<WindowSingletonComponent>();
    const auto& renderingContextComponent = mWorld.GetSingletonComponent<RenderingContextSingletonComponent>();

    while (!AppShouldQuit())
    {
        // Calculate frame delta
        const auto currentTicks = static_cast<float>(SDL_GetTicks());
        auto lastFrameTicks     = currentTicks - elapsedTicks;
        elapsedTicks            = currentTicks;
        const auto dt           = lastFrameTicks * 0.001f;

        framesAccumulator++;
        dtAccumulator += dt;

        if (dtAccumulator > 1.0f)
        {
            const auto fpsString            = " - FPS: " + std::to_string(framesAccumulator);
            const auto frustumCulledString  = " - FCed: " + std::to_string(renderingContextComponent.mFrustumCulledEntities);
            const auto entityCountString    = " - Entities: " + std::to_string(mWorld.GetActiveEntities().size());
            const auto renderingCallsString = " - Render Calls: " + std::to_string(renderingContextComponent.mRenderedEntities);
            SDL_SetWindowTitle(windowComponent.mWindowHandle, (windowComponent.mWindowTitle + fpsString + frustumCulledString + entityCountString + renderingCallsString).c_str());

            framesAccumulator = 0;
            dtAccumulator = 0.0f;
        }

        // Simulate world. Limit dt passed in to be at most 
        // equivalent to running the game at 1 frame a second
        mWorld.Update(math::Min(dt, 1.0f));
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

void App::DummyInitialization()
{
    const auto playerEntity = mWorld.CreateEntity();
    
    const auto levelEntityId  = LoadAndCreateLevelByName(StringId("route1"), mWorld);
    auto& levelModelComponent = mWorld.GetComponent<LevelModelComponent>(levelEntityId);

    auto activeLevelComponent = std::make_unique<ActiveLevelSingletonComponent>();
    activeLevelComponent->mActiveLevelNameId = levelModelComponent.mLevelName;
    mWorld.SetSingletonComponent<ActiveLevelSingletonComponent>(std::move(activeLevelComponent));

    {
        auto animationComponent = std::make_unique<AnimationTimerComponent>();
        animationComponent->mAnimationTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);
        animationComponent->mAnimationTimer->Pause();
        
        mWorld.AddComponent<AnimationTimerComponent>(playerEntity, std::move(animationComponent));
        mWorld.AddComponent<DirectionComponent>(playerEntity, std::make_unique<DirectionComponent>());
        mWorld.AddComponent<MovementStateComponent>(playerEntity, std::make_unique<MovementStateComponent>());
        mWorld.AddComponent<PlayerTagComponent>(playerEntity, std::make_unique<PlayerTagComponent>());
        mWorld.AddComponent<RenderableComponent>(playerEntity, CreateRenderableComponentForSprite(CharacterSpriteData(CharacterMovementType::DYNAMIC, 6, 14)));
        mWorld.AddComponent<TransformComponent>(playerEntity, std::make_unique<TransformComponent>());
    }

    auto& playerTransformComponent = mWorld.GetComponent<TransformComponent>(playerEntity);
    auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(playerEntity);
    
    playerTransformComponent.mPosition = TileCoordsToPosition(16, 16);
    playerMovementStateComponent.mCurrentCoords = TileCoords(16, 16);
    GetTile(16, 16, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = playerEntity;
    GetTile(16, 16, levelModelComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::PLAYER;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
