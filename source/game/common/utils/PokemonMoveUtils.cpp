//
//  PokemonMoveUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonMoveUtils.h"
#include "../GameConstants.h"
#include "../components/MoveStatsSingletonComponent.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <json.hpp>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string MOVE_STATS_FILE_NAME = "moves.json";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

size_t GetFindFirstUnusedMoveIndex(const PokemonMoveSet& moveset)
{
    for (auto i = 0U; i < moveset.size(); ++i)
    {
        if (moveset[i] == nullptr)
        {
            return i;
        }
    }

    return moveset.size();
}

void LoadAndPopulateMoveStats(MoveStatsSingletonComponent& moveStatsComponent)
{
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    // Get level data file resource
    const auto levelFilePath = ResourceLoadingService::RES_DATA_ROOT + MOVE_STATS_FILE_NAME;
    resourceLoadingService.LoadResource(levelFilePath);
    const auto& levelFileResource = resourceLoadingService.GetResource<DataFileResource>(levelFilePath);

    // Parse level json
    const auto levelJson = nlohmann::json::parse(levelFileResource.GetContents());

    for (auto it = levelJson.begin(); it != levelJson.end(); ++it)
    {
        const auto moveName = StringId(it.key());
        const auto& statsObject = it.value();

        moveStatsComponent.mMoveStats.insert(std::make_pair(moveName, PokemonMoveStats
        (
            moveName,
            StringId(statsObject["type"].get<std::string>()),
            statsObject["power"].get<int>(),
            statsObject["accuracy"].get<int>(),
            statsObject["pp"].get<int>()
        )));
    }
}

const PokemonMoveStats& GetMoveStats
(
    const StringId moveName,
    const ecs::World& world
)
{
    const auto& moveStatsComponent = world.GetSingletonComponent<MoveStatsSingletonComponent>();
    return moveStatsComponent.mMoveStats.at(moveName);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

