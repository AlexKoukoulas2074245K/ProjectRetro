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
#include "../components/TransitionAnimationStateSingletonComponent.h"
#include "../components/WarpConnectionsSingletonComponent.h"
#include "../utils/LevelUtils.h"
#include "../utils/LevelLoadingUtils.h"
#include "../../common/components/PlayerTagComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../resources/DataFileResource.h"

#include <json.hpp>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string WARP_CONNECTIONS_FILE_NAME = "warp_connections.json";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

WarpConnectionsSystem::WarpConnectionsSystem(ecs::World& world)
    : BaseSystem(world)
{
    PopulateWarpConnections();
    CalculateAndSetComponentUsageMask<PlayerTagComponent>();
}

void WarpConnectionsSystem::VUpdateAssociatedComponents(const float) const
{
    const auto& transitionAnimationStateSingletonComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    auto& activeLevelSingletonComponent                    = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    auto& warpConnectionsComponent                         = mWorld.GetSingletonComponent<WarpConnectionsSingletonComponent>();

    
    if (warpConnectionsComponent.mHasPendingWarpConnection && transitionAnimationStateSingletonComponent.mIsPlayingTransitionAnimation == false)
    {        
        DestroyCurrentLevel(activeLevelSingletonComponent.mActiveLevelNameId);
        
        const auto playerEntityId = GetOverworldPlayerEntityId(mWorld);
        const auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(playerEntityId);

        WarpInfo currentWarp
        (
            activeLevelSingletonComponent.mActiveLevelNameId.GetString(),
            TileCoords(playerMovementStateComponent.mCurrentCoords.mCol, playerMovementStateComponent.mCurrentCoords.mRow)
        );

        assert(warpConnectionsComponent.mWarpConnections.count(currentWarp) != 0 &&
            "Warp for current tile not found");

        auto& targetWarp = warpConnectionsComponent.mWarpConnections.at(currentWarp);
        targetWarp.mLevelName = StringId();
        /*
        activeLevelSingletonComponent.mActiveLevelNameId = StringId("in_players_home_top");

        auto& newLevelModelComponent = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(StringId("in_players_home_top"), mWorld));

        auto& playerTransformComponent = mWorld.GetComponent<TransformComponent>(1);
        auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(1);

        playerTransformComponent.mPosition = TileCoordsToPosition(8, 10);
        playerMovementStateComponent.mCurrentCoords = TileCoords(8, 10);
        GetTile(8, 10, newLevelModelComponent.mLevelTilemap).mTileOccupierEntityId = 5;
        GetTile(8, 10, newLevelModelComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::PLAYER;

        warpConnectionsComponent.mHasPendingWarpConnection = false;
        */
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void WarpConnectionsSystem::PopulateWarpConnections() const
{
    // Load warp connections file
    const auto warpConnectionsResourceId = ResourceLoadingService::GetInstance().
        LoadResource(ResourceLoadingService::RES_DATA_ROOT + WARP_CONNECTIONS_FILE_NAME);

    const auto& warpConnectionsFileResource = ResourceLoadingService::GetInstance().
        GetResource<DataFileResource>(warpConnectionsResourceId);

    // Parse warp connections file
    auto warpConnectionsComponent = std::make_unique<WarpConnectionsSingletonComponent>();
    const auto warpConnecionsJson = nlohmann::json::parse(warpConnectionsFileResource.GetContents());     

    for (const auto& connectionJsonObject: warpConnecionsJson["connections"])
    {
        const auto& fromWarpJsonObject = connectionJsonObject["from"];
        const auto& toWarpJsonObject   = connectionJsonObject["to"];

        WarpInfo fromWarpInfo
        (
            fromWarpJsonObject["level_name"].get<std::string>(), 
            TileCoords(fromWarpJsonObject["level_col"].get<int>(), fromWarpJsonObject["level_row"].get<int>())
        );

        WarpInfo toWarpInfo
        (
            toWarpJsonObject["level_name"].get<std::string>(),
            TileCoords(toWarpJsonObject["level_col"].get<int>(), toWarpJsonObject["level_row"].get<int>())
        );

        warpConnectionsComponent->mWarpConnections[fromWarpInfo] = toWarpInfo;
    }

    mWorld.SetSingletonComponent<WarpConnectionsSingletonComponent>(std::move(warpConnectionsComponent));
}

void WarpConnectionsSystem::DestroyCurrentLevel(const StringId levelNameId) const
{    
    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (mWorld.HasComponent<LevelResidentComponent>(entityId))
        {
            if (mWorld.GetComponent<LevelResidentComponent>(entityId).mLevelNameId == levelNameId)
            {
                mWorld.RemoveEntity(entityId);
            }
        }
    }
    
    mWorld.RemoveEntity(GetLevelIdFromNameId(levelNameId, mWorld));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
