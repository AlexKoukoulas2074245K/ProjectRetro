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
#include "../utils/LevelLoadingUtils.h"
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

        activeLevelSingletonComponent.mActiveLevelNameId = StringId("in_players_home_top");

        auto& newLevelModelComponent = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(StringId("in_players_home_top"), mWorld));

        auto& playerTransformComponent = mWorld.GetComponent<TransformComponent>(1);
        auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(1);

        playerTransformComponent.mPosition = TileCoordsToPosition(8, 10);
        playerMovementStateComponent.mCurrentCoords = TileCoords(8, 10);
        GetTile(8, 10, newLevelModelComponent.mLevelTilemap).mTileOccupierEntityId = 5;
        GetTile(8, 10, newLevelModelComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::PLAYER;

        warpConnectionsComponent.mHasPendingWarpConnection = false;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
