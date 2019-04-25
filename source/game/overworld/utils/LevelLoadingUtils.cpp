//
//  LevelLoadingUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/04/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "LevelLoadingUtils.h"
#include "LevelUtils.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <json.hpp>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId LoadAndCreateLevelByName(ecs::World& world, const StringId levelName)
{
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    
    // Get level data file resource
    const auto levelFilePath = ResourceLoadingService::RES_DATA_ROOT + levelName.GetString() + ".json";
    if (resourceLoadingService.HasLoadedResource(levelFilePath) == false)
    {
        resourceLoadingService.LoadResource(levelFilePath);
    }
    const auto& levelFileResource = resourceLoadingService.GetResource<DataFileResource>(levelFilePath);    

    // Parse level json
    const auto levelJson = nlohmann::json::parse(levelFileResource.GetContents());

    // Extract level header properties
    const auto levelName        = levelJson["level_header"]["name"].get<std::string>();
    const auto levelTilemapCols = levelJson["level_header"]["dimensions"]["cols"].get<int>();
    const auto levelTilemapRows = levelJson["level_header"]["dimensions"]["rows"].get<int>();

    // Initialize result level entity and context component
    auto levelEntityId                   = world.CreateEntity();
    auto levelContextComponent           = std::make_unique<LevelContextComponent>();
    levelContextComponent->mLevelName    = StringId(levelName);
    levelContextComponent->mLevelTilemap = InitializeTilemapWithDimensions(levelTilemapCols, levelTilemapRows);

    world.AddComponent<LevelContextComponent>(levelEntityId, std::move(levelContextComponent));
    return levelEntityId;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

