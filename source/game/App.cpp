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
#include "common/utils/MathUtils.h"
#include "common/components/PlayerTagComponent.h"
#include "input/components/InputStateSingletonComponent.h"
#include "input/systems/RawInputHandlingSystem.h"
#include "rendering/components/AnimationTimerComponent.h"
#include "rendering/components/RenderableComponent.h"
#include "rendering/components/RenderingContextSingletonComponent.h"
#include "rendering/components/WindowSingletonComponent.h"
#include "rendering/systems/AnimationSystem.h"
#include "rendering/systems/CameraControlSystem.h"
#include "rendering/systems/RenderingSystem.h"
#include "resources/ResourceLoadingService.h"
#include "resources/TextureUtils.h"
#include "overworld/components/ActiveLevelSingletonComponent.h"
#include "overworld/components/LevelResidentComponent.h"
#include "overworld/components/LevelContextComponent.h"
#include "overworld/components/MovementStateComponent.h"
#include "overworld/components/WarpConnectionsSingletonComponent.h"
#include "overworld/systems/MovementControllerSystem.h"
#include "overworld/systems/PlayerActionControllerSystem.h"
#include "overworld/systems/WarpConnectionsSystem.h"
#include "overworld/utils/LevelUtils.h"
#include "overworld/utils/LevelLoadingUtils.h"

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

    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + "characters.png");
    renderableComponent->mActiveAnimationNameId = NORTH_ANIMATION_NAME_ID;
    renderableComponent->mShaderNameId          = StringId("basic");
    renderableComponent->mAffectedByPerspective = false;
    
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
    mWorld.AddSystem(std::make_unique<RawInputHandlingSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<PlayerActionControllerSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<AnimationSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<MovementControllerSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<WarpConnectionsSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<CameraControlSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<RenderingSystem>(mWorld));
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
            const auto fpsString = " - FPS: " + std::to_string(framesAccumulator);
            const auto frustumCulledString = " - FCed: " + std::to_string(renderingContextComponent.mFrustumCulledEntities);
            SDL_SetWindowTitle(windowComponent.mWindowHandle, (windowComponent.mWindowTitle + fpsString + frustumCulledString).c_str());

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
    const auto testLevelAEntityId = mWorld.CreateEntity();
    const auto testLevelBEntityId = mWorld.CreateEntity();

    const auto otherDummyEntity = mWorld.CreateEntity();
    const auto dummyEntity = mWorld.CreateEntity();
    const auto playerEntity = mWorld.CreateEntity();

    const auto levelEntityId =LoadAndCreateLevelByName(StringId("pallet_new"), mWorld);
    auto& levelContextComponent = mWorld.GetComponent<LevelContextComponent>(levelEntityId);

    //GetTile(10, 13, levelContextComponent.mLevelTilemap).mTileTrait = TileTrait::WARP;
    
    auto activeLevelComponent = std::make_unique<ActiveLevelSingletonComponent>();
    activeLevelComponent->mActiveLevelNameId = levelContextComponent.mLevelName;
    mWorld.SetSingletonComponent<ActiveLevelSingletonComponent>(std::move(activeLevelComponent));

    {
        auto levelResidentComponent = std::make_unique<LevelResidentComponent>();
        levelResidentComponent->mLevelNameId = levelContextComponent.mLevelName;

        mWorld.AddComponent<TransformComponent>(dummyEntity, std::make_unique<TransformComponent>()); 
        mWorld.AddComponent<LevelResidentComponent>(dummyEntity, std::move(levelResidentComponent));
        mWorld.AddComponent<RenderableComponent>(dummyEntity, CreateRenderableComponentForSprite(SpriteData(SpriteType::DYNAMIC, 0, 0)));
    }

    {
        auto levelResidentComponent = std::make_unique<LevelResidentComponent>();
        levelResidentComponent->mLevelNameId = levelContextComponent.mLevelName;

        mWorld.AddComponent<TransformComponent>(otherDummyEntity, std::make_unique<TransformComponent>());
        mWorld.AddComponent<LevelResidentComponent>(otherDummyEntity, std::move(levelResidentComponent));
        mWorld.AddComponent<RenderableComponent>(otherDummyEntity, CreateRenderableComponentForSprite(SpriteData(SpriteType::DYNAMIC, 0, 0)));
    }

    {
        auto animationComponent = std::make_unique<AnimationTimerComponent>();
        animationComponent->mAnimationTimer = std::make_unique<Timer>(0.125f);
        animationComponent->mAnimationTimer->Pause();
        
        mWorld.AddComponent<AnimationTimerComponent>(playerEntity, std::move(animationComponent));
        mWorld.AddComponent<DirectionComponent>(playerEntity, std::make_unique<DirectionComponent>());
        mWorld.AddComponent<MovementStateComponent>(playerEntity, std::make_unique<MovementStateComponent>());
        mWorld.AddComponent<PlayerTagComponent>(playerEntity, std::make_unique<PlayerTagComponent>());
        mWorld.AddComponent<RenderableComponent>(playerEntity, CreateRenderableComponentForSprite(SpriteData(SpriteType::DYNAMIC, 6, 14)));
        mWorld.AddComponent<TransformComponent>(playerEntity, std::make_unique<TransformComponent>());
    }

    auto& playerTransformComponent = mWorld.GetComponent<TransformComponent>(playerEntity);
    auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(playerEntity);
    
    playerTransformComponent.mPosition = TileCoordsToPosition(0, 0);
    playerMovementStateComponent.mCurrentCoords = TileCoords(0, 0);
    GetTile(0, 0, levelContextComponent.mLevelTilemap).mTileOccupierEntityId = playerEntity;
    GetTile(0, 0, levelContextComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::PLAYER;

    auto& dummyTransformComponent = mWorld.GetComponent<TransformComponent>(dummyEntity);
    dummyTransformComponent.mPosition = TileCoordsToPosition(20, 16);
    GetTile(20, 16, levelContextComponent.mLevelTilemap).mTileOccupierEntityId = dummyEntity;
    GetTile(20, 16, levelContextComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::NPC;

    auto& otherDummyTransformComponent = mWorld.GetComponent<TransformComponent>(otherDummyEntity);
    otherDummyTransformComponent.mPosition = TileCoordsToPosition(10, 12);
    GetTile(10, 12, levelContextComponent.mLevelTilemap).mTileOccupierEntityId = otherDummyEntity;
    GetTile(10, 12, levelContextComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::NPC;

}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
