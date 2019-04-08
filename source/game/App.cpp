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
#include "common/components/DirectionComponent.h"
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
#include "resources/TextureUtils.h"
#include "overworld/components/LevelGridComponent.h"
#include "overworld/components/MovementStateComponent.h"
#include "overworld/systems/MovementControllerSystem.h"
#include "overworld/systems/PlayerActionControllerSystem.h"
#include "overworld/utils/LevelUtils.h"

#include <SDL_events.h> 
#include <SDL_timer.h>  

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//TEMP move to levelloading flow when implemented 
//
static const StringId NORTH_ANIMATION_NAME_ID = StringId("north");
static const StringId SOUTH_ANIMATION_NAME_ID = StringId("south");
static const StringId WEST_ANIMATION_NAME_ID = StringId("west");
static const StringId EAST_ANIMATION_NAME_ID = StringId("east");

enum class SpriteType
{
    DYNAMIC, STATIONARY, STATIC
};

class SpriteData
{
public:
    SpriteData(const SpriteType spriteType, const int atlasColOffset, const int atlasRowOffset)
        : mSpriteType(spriteType)
        , mAtlasColOffset(atlasColOffset)
        , mAtlasRowOffset(atlasRowOffset)
    {
    }

    const SpriteType mSpriteType;
    const int mAtlasColOffset;
    const int mAtlasRowOffset;
};

std::string CreateTexCoordInjectedModelPath(const std::string& modelName, const std::vector<glm::vec2>& texCoords)
{
    std::string path = ResourceLoadingService::RES_MODELS_ROOT + modelName + "[";
    
    path += std::to_string(texCoords[0].x) + "," + std::to_string(texCoords[0].y);

    for (auto i = 1U; i < texCoords.size(); ++i)
    {
        path += "-" + std::to_string(texCoords[i].x) + "," + std::to_string(texCoords[i].y);        
    }
    
    path += "].obj";

    return path;
}

void LoadMeshFromTexCoordsAndAddToRenderableComponent(const int meshAtlasCol, const int meshAtlasRow, const bool horFlipped, const StringId& animationNameId, RenderableComponent& renderableComponent)
{
    const auto atlasCols = 8;
    const auto atlasRows = 64;

    auto correctedMeshCol = meshAtlasCol;
    auto correctedMeshRow = meshAtlasRow;

    if (correctedMeshCol >= atlasCols)
    {
        correctedMeshCol %= atlasCols;
        correctedMeshRow++;
    }

    const auto texCoords = CalculateTextureCoordsFromColumnAndRow(correctedMeshCol, correctedMeshRow, atlasCols, atlasRows, horFlipped);
    const auto meshPath  = CreateTexCoordInjectedModelPath("camera_facing_quad", texCoords);

    renderableComponent.mAnimationsToMeshes[animationNameId].push_back(ResourceLoadingService::GetInstance().LoadResource(meshPath));
}

std::unique_ptr<RenderableComponent> CreateRenderableComponentForSprite(const SpriteData& spriteData)
{    
    auto renderableComponent = std::make_unique<RenderableComponent>();    

    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource("textures/materials/overworld.png");
    renderableComponent->mActiveAnimationNameId = NORTH_ANIMATION_NAME_ID;
    renderableComponent->mShaderNameId          = StringId("basic");
            
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset, spriteData.mAtlasRowOffset, false, SOUTH_ANIMATION_NAME_ID, *renderableComponent);

    if (spriteData.mSpriteType == SpriteType::STATIC) return renderableComponent;

    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 1, spriteData.mAtlasRowOffset, false, NORTH_ANIMATION_NAME_ID, *renderableComponent);
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 2, spriteData.mAtlasRowOffset, false, WEST_ANIMATION_NAME_ID, *renderableComponent);
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 2, spriteData.mAtlasRowOffset, true, EAST_ANIMATION_NAME_ID, *renderableComponent);

    if (spriteData.mSpriteType == SpriteType::STATIONARY) return renderableComponent;

    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 3, spriteData.mAtlasRowOffset, true, SOUTH_ANIMATION_NAME_ID, *renderableComponent);
    renderableComponent->mAnimationsToMeshes[SOUTH_ANIMATION_NAME_ID].push_back(renderableComponent->mAnimationsToMeshes[SOUTH_ANIMATION_NAME_ID][0]);
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 3, spriteData.mAtlasRowOffset, false, SOUTH_ANIMATION_NAME_ID, *renderableComponent);

    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 4, spriteData.mAtlasRowOffset, true, NORTH_ANIMATION_NAME_ID, *renderableComponent);
    renderableComponent->mAnimationsToMeshes[NORTH_ANIMATION_NAME_ID].push_back(renderableComponent->mAnimationsToMeshes[NORTH_ANIMATION_NAME_ID][0]);
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 4, spriteData.mAtlasRowOffset, false, NORTH_ANIMATION_NAME_ID, *renderableComponent);

    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 5, spriteData.mAtlasRowOffset, false, WEST_ANIMATION_NAME_ID, *renderableComponent);
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 5, spriteData.mAtlasRowOffset, true, EAST_ANIMATION_NAME_ID, *renderableComponent);

    return renderableComponent;
}

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
    mWorld.SetSystem<PlayerActionControllerSystem>(std::make_unique<PlayerActionControllerSystem>(mWorld));
    mWorld.SetSystem<AnimationSystem>(std::make_unique<AnimationSystem>(mWorld));
    mWorld.SetSystem<MovementControllerSystem>(std::make_unique<MovementControllerSystem>(mWorld));
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
    const auto playerEntity = mWorld.CreateEntity();
    
    auto levelGridComponent = std::make_unique<LevelGridComponent>();
    levelGridComponent->mRows = 5U;
    levelGridComponent->mCols = 5U;
    levelGridComponent->mLevelGrid = InitializeLevelGridOfDimensions(levelGridComponent->mCols, levelGridComponent->mRows);
    
    mWorld.SetSingletonComponent<LevelGridComponent>(std::move(levelGridComponent));    

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
    transformComponent2->mPosition = LevelTileCoordsToPosition(TileCoords(2, 2));

    auto animationComponent = std::make_unique<AnimationTimerComponent>();
    animationComponent->mAnimationTimer = std::make_unique<Timer>(0.125f);
    animationComponent->mAnimationTimer->Pause();

    auto movementStateComponent = std::make_unique<MovementStateComponent>();
    movementStateComponent->mCurrentCoords = TileCoords(2, 2);
    movementStateComponent->mTargetCoords  = TileCoords(2, 2);

    mWorld.AddComponent<AnimationTimerComponent>(playerEntity, std::move(animationComponent));
    mWorld.AddComponent<DirectionComponent>(playerEntity, std::make_unique<DirectionComponent>());
    mWorld.AddComponent<MovementStateComponent>(playerEntity, std::move(movementStateComponent));
    mWorld.AddComponent<PlayerTagComponent>(playerEntity, std::make_unique<PlayerTagComponent>());
    mWorld.AddComponent<RenderableComponent>(playerEntity, CreateRenderableComponentForSprite(SpriteData(SpriteType::DYNAMIC, 6, 14)));
    mWorld.AddComponent<TransformComponent>(playerEntity, std::move(transformComponent2));
    
    bool topRightTexture = true;
    for (int y = 0; y < 5; ++y)
    {
        for (int x = 0; x < 5; ++x)
        {
            auto tileEntity = mWorld.CreateEntity();
            auto tileTransformComponent = std::make_unique<TransformComponent>();
            tileTransformComponent->mPosition = glm::vec3(x * OVERWORLD_TILE_SIZE, -OVERWORLD_TILE_SIZE * 0.5f, y * OVERWORLD_TILE_SIZE);

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
