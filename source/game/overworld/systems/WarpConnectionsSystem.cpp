//
//  WarpConnectionsSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "WarpConnectionsSystem.h"
#include "../components/ActiveLevelSingletonComponent.h"
#include "../components/LevelResidentComponent.h"
#include "../components/WarpConnectionsSingletonComponent.h"
#include "../utils/LevelUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

WarpConnectionsSystem::WarpConnectionsSystem(ecs::World& world)
    : BaseSystem(world)
{
    mWorld.SetSingletonComponent<WarpConnectionsSingletonComponent>(std::make_unique<WarpConnectionsSingletonComponent>());
}

void WarpConnectionsSystem::VUpdateAssociatedComponents(const float) const
{
    auto& activeLevelSingletonComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    auto& warpConnectionsComponent      = mWorld.GetSingletonComponent<WarpConnectionsSingletonComponent>();

    if (warpConnectionsComponent.mHasPendingWarpConnection)
    {
        for (const auto& entityId : mWorld.GetActiveEntities())
        {
            if (mWorld.HasComponent<LevelResidentComponent>(entityId))
            {
                if (mWorld.GetComponent<LevelResidentComponent>(entityId).mLevelNameId == activeLevelSingletonComponent.mActiveLevelNameId)
                {
                    mWorld.RemoveEntity(entityId);
                }
            }
        }

        mWorld.RemoveEntity(GetLevelIdFromNameId(activeLevelSingletonComponent.mActiveLevelNameId, mWorld));

        activeLevelSingletonComponent.mActiveLevelNameId = StringId("testLevelB");
        auto& newLevelContextComponent = mWorld.GetComponent<LevelContextComponent>(GetLevelIdFromNameId(StringId("testLevelB"), mWorld));
        auto& playerTransformComponent = mWorld.GetComponent<TransformComponent>(5);
        auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(5);

        {
            const auto levelGroundLayer = mWorld.CreateEntity();
            auto transformComponent = std::make_unique<TransformComponent>();
            transformComponent->mScale.x = 5.0f;
            transformComponent->mScale.z = 5.0f;
            transformComponent->mPosition.x += (5.0f * OVERWORLD_TILE_SIZE) / 2.0f;
            transformComponent->mPosition.y -= OVERWORLD_TILE_SIZE / 2.0f;
            transformComponent->mPosition.z += (5.0f * OVERWORLD_TILE_SIZE) / 2.0f;

            auto renderableComponent = std::make_unique<RenderableComponent>();
            renderableComponent->mShaderNameId = StringId("basic");
            renderableComponent->mAnimationsToMeshes[StringId("default")].
                push_back(ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_MODELS_ROOT + "2d_out_empty_floor.obj"));
            renderableComponent->mActiveAnimationNameId = StringId("default");
            renderableComponent->mTextureResourceId =
                ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_TEXTURES_ROOT + "materials/pallet_ground_layer.png");
            renderableComponent->mRenderableLayer = RenderableLayer::LEVEL_FLOOR_LEVEL;

            auto levelResidentComponent = std::make_unique<LevelResidentComponent>();
            levelResidentComponent->mLevelNameId = newLevelContextComponent.mLevelName;

            mWorld.AddComponent<TransformComponent>(levelGroundLayer, std::move(transformComponent));
            mWorld.AddComponent<LevelResidentComponent>(levelGroundLayer, std::move(levelResidentComponent));
            mWorld.AddComponent<RenderableComponent>(levelGroundLayer, std::move(renderableComponent));
        }

        playerTransformComponent.mPosition = TileCoordsToPosition(2, 2);
        playerMovementStateComponent.mCurrentCoords = TileCoords(2, 2);
        GetTile(2, 2, newLevelContextComponent.mLevelTilemap).mTileOccupierEntityId = 5;
        GetTile(2, 2, newLevelContextComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::PLAYER;

        warpConnectionsComponent.mHasPendingWarpConnection = false;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////