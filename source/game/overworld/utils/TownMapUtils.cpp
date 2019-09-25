//
//  TownMapUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 24/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TownMapUtils.h"
#include "../components/TownMapLocationDataSingletonComponent.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../rendering/components/WindowSingletonComponent.h"
#include "../../common/utils/MathUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <json.hpp>
#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const glm::vec3 TOWN_MAP_BACKGROUND_POSITION           = glm::vec3(0.0f, -0.01f, 0.1f);
static const glm::vec3 TOWN_MAP_BACKGROUND_SCALE              = glm::vec3(2.2f, 2.2f, 1.0f);
static const glm::vec3 TOWN_MAP_ICON_TOP_LEFT_ORIGIN_POSITION = glm::vec3(-0.62f, 0.885f, 0.0f);

static const std::string TOWN_MAP_LOCATIONS_DATA_FILE_NAME      = "town_map_locations.json";
static const std::string TOWN_MAP_SPRITE_MODEL_FILE_NAME        = "town_map_sprite.obj";
static const std::string TOWN_MAP_BACKGROUND_TEXTURE_FILE_NAME  = "town_map_background.png";
static const std::string TOWN_MAP_PLAYER_ICON_TEXTURE_FILE_NAME = "town_map_player_icon.png";
static const std::string TOWN_MAP_CURSOR_ICON_TEXTURE_FILE_NAME = "town_map_cursor_icon.png";

static const float TOWN_MAP_ICON_Z = 0.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId LoadAndCreateTownMapBackground
(
    ecs::World& world
)
{
    const auto backgroundEntityId = world.CreateEntity();

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    auto renderableComponent = std::make_unique<RenderableComponent>();
    
    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + TOWN_MAP_BACKGROUND_TEXTURE_FILE_NAME;
    renderableComponent->mTextureResourceId     = resourceLoadingService.LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;

    const auto modelPath = ResourceLoadingService::RES_MODELS_ROOT + TOWN_MAP_SPRITE_MODEL_FILE_NAME;    
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = TOWN_MAP_BACKGROUND_POSITION;
    transformComponent->mScale    = TOWN_MAP_BACKGROUND_SCALE;

    world.AddComponent<RenderableComponent>(backgroundEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(backgroundEntityId, std::move(transformComponent));

    return backgroundEntityId;
}

ecs::EntityId LoadAndCreateTownMapIconAtLocation
(
    const TownMapIconType iconType,
    const StringId location,
    ecs::World& world
)
{
    const auto& guiStateComponent             = world.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& townMapLocationsDataComponent = world.GetSingletonComponent<TownMapLocationDataSingletonComponent>();    
    const auto& windowComponent               = world.GetSingletonComponent<WindowSingletonComponent>();
    const auto townMapIconEntityId = world.CreateEntity();

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    auto renderableComponent = std::make_unique<RenderableComponent>();

    auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT;
    switch (iconType)
    {
        case TownMapIconType::PLAYER_ICON: texturePath += TOWN_MAP_PLAYER_ICON_TEXTURE_FILE_NAME; break;
        case TownMapIconType::CURSOR_ICON: texturePath += TOWN_MAP_CURSOR_ICON_TEXTURE_FILE_NAME; break;
    }
    
    renderableComponent->mTextureResourceId     = resourceLoadingService.LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;

    const auto modelPath = ResourceLoadingService::RES_MODELS_ROOT + TOWN_MAP_SPRITE_MODEL_FILE_NAME;
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    glm::vec3 locationCoords;    
    if (townMapLocationsDataComponent.mTownMapLocationsToPositions.count(location) != 0)
    {
        locationCoords = townMapLocationsDataComponent.mTownMapLocationsToPositions.at(location);
    }
    else
    {
        locationCoords = townMapLocationsDataComponent.mTownMapLocationsToPositions.at(townMapLocationsDataComponent.mIndoorLocationsToOwnerLevels.at(location));
    }
    
    auto transformedCoords = TOWN_MAP_ICON_TOP_LEFT_ORIGIN_POSITION;
    transformedCoords.x += GUI_PIXEL_SIZE * locationCoords.x;
    transformedCoords.y -= GUI_PIXEL_SIZE * locationCoords.y * windowComponent.mAspectRatio;

    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = glm::vec3(transformedCoords.x, transformedCoords.y, TOWN_MAP_ICON_Z);
    transformComponent->mScale    = glm::vec3(guiStateComponent.mGlobalGuiTileWidth * 2, guiStateComponent.mGlobalGuiTileHeight * 2, 1.0f);

    world.AddComponent<RenderableComponent>(townMapIconEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(townMapIconEntityId, std::move(transformComponent));

    return townMapIconEntityId;
}

void LoadAndPopulateTownMapLocationData
(
    TownMapLocationDataSingletonComponent& townMapDataComponent
)
{
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    // Get pokemon town map locations data file resource
    const auto townMapDataFilePath = ResourceLoadingService::RES_DATA_ROOT + TOWN_MAP_LOCATIONS_DATA_FILE_NAME;
    resourceLoadingService.LoadResource(townMapDataFilePath);
    const auto& townMapDataFileResource = resourceLoadingService.GetResource<DataFileResource>(townMapDataFilePath);

    // Parse town map locations json
    const auto townMapDataJson = nlohmann::json::parse(townMapDataFileResource.GetContents());

    // Parse indoor locations to owner levels mapping data
    auto& indoorLocationsToLevelsJson = townMapDataJson["indoor_locations_to_owner_levels"];    
    for (auto it = indoorLocationsToLevelsJson.begin(); it != indoorLocationsToLevelsJson.end(); ++it)
    {
        const auto& indoorLocationName = StringId(it.value()["indoor_location_name"].get<std::string>());
        const auto& ownerLevelName     = StringId(it.value()["owner_level_name"].get<std::string>());

        townMapDataComponent.mIndoorLocationsToOwnerLevels[indoorLocationName] = ownerLevelName;
    }

    // Parse town map locations positions 
    auto& townMapLocationsJson = townMapDataJson["location_coords"];    
    for (auto it = townMapLocationsJson.begin(); it != townMapLocationsJson.end(); ++it)
    {
        const auto& locationDataJson = it.value();
        const auto& locationName = locationDataJson["level_name"].get<std::string>();
        const auto mapPosition = glm::vec3(locationDataJson["map_position_x"].get<int>(), locationDataJson["map_position_y"].get<int>(), TOWN_MAP_ICON_Z);

        townMapDataComponent.mTownMapLocationsToPositions[StringId(locationName)] = mapPosition;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
