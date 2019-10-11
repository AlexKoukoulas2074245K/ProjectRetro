//
//  OverworldUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OverworldUtils.h"
#include "../components/ActiveLevelSingletonComponent.h"
#include "../components/WarpConnectionsSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void SetCurrentPokeCenterAsHome(ecs::World& world)
{
	const auto& activeLevelComponent          = world.GetSingletonComponent<ActiveLevelSingletonComponent>();
	const auto& townMapLocationsDataComponent = world.GetSingletonComponent<TownMapLocationDataSingletonComponent>();
	const auto& warpConnectionsComponent      = world.GetSingletonComponent<WarpConnectionsSingletonComponent>();
	auto& playerStateComponent                = world.GetSingletonComponent<PlayerStateSingletonComponent>();

	playerStateComponent.mHomeLevelName = townMapLocationsDataComponent.mIndoorLocationsToOwnerLocations.at(activeLevelComponent.mActiveLevelNameId);

	// Find warp from town to this poke center and extract tile coords of entrance to center
	for (const auto& warpEntry : warpConnectionsComponent.mWarpConnections)
	{
		const auto& fromWarp = warpEntry.first;
		const auto& toWarp   = warpEntry.second;

		if (fromWarp.mLevelName == playerStateComponent.mHomeLevelName && toWarp.mLevelName == activeLevelComponent.mActiveLevelNameId)
		{
			playerStateComponent.mHomeLevelOccupiedCol = fromWarp.mTileCoords.mCol;
			playerStateComponent.mHomeLevelOccupiedRow = fromWarp.mTileCoords.mRow - 1;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
