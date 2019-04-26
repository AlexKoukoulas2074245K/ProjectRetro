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
#include "../components/LevelResidentComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/StringUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <json.hpp>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string LEVEL_GROUND_LAYER_MODEL_FILE_NAME    = "2d_out_empty_floor.obj";
static const std::string LEVEL_GROUND_LAYER_TEXTURE_NAME_TRAIL = "_groundLayer.png";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static void CreateLevelGroundLayer
(
    const StringId levelNameId,
    const int levelCols,
    const int levelRows,
    const float groundLayerX, 
    const float groundLayerZ, 
    const ecs::EntityId groundLayerEntityId,
    ecs::World& world
);

static void CreateLevelModelEntry
(
    const nlohmann::basic_json<>& modelEntryJsonObject, 
    const StringId levelNameId, 
    ecs::World& world
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId LoadAndCreateLevelByName(const StringId levelName, ecs::World& world)
{
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    
    // Get level data file resource
    const auto levelFilePath = ResourceLoadingService::RES_LEVELS_ROOT + levelName.GetString() + ".json";
    if (resourceLoadingService.HasLoadedResource(levelFilePath) == false)
    {
        resourceLoadingService.LoadResource(levelFilePath);
    }
    const auto& levelFileResource = resourceLoadingService.GetResource<DataFileResource>(levelFilePath);    

    // Parse level json
    const auto levelJson = nlohmann::json::parse(levelFileResource.GetContents());

    // Extract level header properties
    const auto levelTilemapCols = levelJson["level_header"]["dimensions"]["cols"].get<int>();
    const auto levelTilemapRows = levelJson["level_header"]["dimensions"]["rows"].get<int>();

    // Initialize result level entity and context component
    const auto levelEntityId                  = world.CreateEntity();
    auto levelContextComponent                = std::make_unique<LevelContextComponent>();

    levelContextComponent->mLevelName         = StringId(levelName);
    levelContextComponent->mLevelTilemap      = InitializeTilemapWithDimensions(levelTilemapCols, levelTilemapRows);
    levelContextComponent->mCols              = levelTilemapCols;
    levelContextComponent->mRows              = levelTilemapRows;
    levelContextComponent->mGroundLayerEntity = world.CreateEntity();

    // Create optimized ground layer texture for level
    const auto groundLayerTextureName = levelJson["level_ground_layer_game"][0]["texture_name"];
    const auto groundLayerPositionX   = levelJson["level_ground_layer_game"][0]["game_position_x"];
    const auto groundLayerPositionZ   = levelJson["level_ground_layer_game"][0]["game_position_z"];

    CreateLevelGroundLayer
    (
        levelContextComponent->mLevelName,
        levelTilemapCols,
        levelTilemapRows,
        groundLayerPositionX,
        groundLayerPositionZ,
        levelContextComponent->mGroundLayerEntity, 
        world
    );
    
    // Todo: Load NPC list

    // Load model list
    for (const auto& modelEntry: levelJson["level_model_list"])
    {
        CreateLevelModelEntry(modelEntry, levelContextComponent->mLevelName, world);
    }

    world.AddComponent<LevelContextComponent>(levelEntityId, std::move(levelContextComponent));
    return levelEntityId;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void CreateLevelGroundLayer
(
    const StringId levelNameId,
    const int levelCols,
    const int levelRows,
    const float groundLayerX,
    const float groundLayerZ,
    const ecs::EntityId groundLayerEntityId, 
    ecs::World& world
)
{    
    auto transformComponent          = std::make_unique<TransformComponent>();
    transformComponent->mScale.x     = static_cast<float>(levelCols);
    transformComponent->mScale.z     = static_cast<float>(levelRows);
    transformComponent->mPosition.x += groundLayerX - OVERWORLD_TILE_SIZE / 2.0f;
    transformComponent->mPosition.y -= OVERWORLD_TILE_SIZE / 2.0f;
    transformComponent->mPosition.z += groundLayerZ - OVERWORLD_TILE_SIZE / 2.0f;

    auto renderableComponent = std::make_unique<RenderableComponent>();
    renderableComponent->mShaderNameId = StringId("basic");
    renderableComponent->mAnimationsToMeshes[StringId("default")].
        push_back(ResourceLoadingService::GetInstance().
        LoadResource(ResourceLoadingService::RES_MODELS_ROOT + 
        LEVEL_GROUND_LAYER_MODEL_FILE_NAME));
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mTextureResourceId =
        ResourceLoadingService::GetInstance().LoadResource(
        ResourceLoadingService::RES_TEXTURES_ROOT + 
        levelNameId.GetString() + 
        LEVEL_GROUND_LAYER_TEXTURE_NAME_TRAIL);
    renderableComponent->mRenderableLayer = RenderableLayer::LEVEL_FLOOR_LEVEL;

    auto levelResidentComponent = std::make_unique<LevelResidentComponent>();
    levelResidentComponent->mLevelNameId = levelNameId;

    world.AddComponent<TransformComponent>(groundLayerEntityId, std::move(transformComponent));
    world.AddComponent<LevelResidentComponent>(groundLayerEntityId, std::move(levelResidentComponent));
    world.AddComponent<RenderableComponent>(groundLayerEntityId, std::move(renderableComponent));
}

void CreateLevelModelEntry
(
    const nlohmann::basic_json<>& modelEntryJsonObject, 
    const StringId levelNameId, 
    ecs::World& world
)
{
    const auto modelEntityId = world.CreateEntity();

    auto transformComponent         = std::make_unique<TransformComponent>();
    transformComponent->mPosition.x = modelEntryJsonObject["game_position_x"].get<float>();
    transformComponent->mPosition.y = 0.0f;
    transformComponent->mPosition.z = modelEntryJsonObject["game_position_z"].get<float>();

    // Extract model name from the: 'model_name (col_dim, row_dim)' format
    const auto modelName = StringSplit(modelEntryJsonObject["model_name"].get<std::string>(), ' ')[0];

    auto renderableComponent = std::make_unique<RenderableComponent>();
    renderableComponent->mShaderNameId = StringId("basic");
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back
    (
        ResourceLoadingService::GetInstance().
        LoadResource(ResourceLoadingService::RES_MODELS_ROOT + modelName + ".obj"
    ));
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource
    (
        ResourceLoadingService::RES_TEXTURES_ROOT + modelName + ".png"
    );
    renderableComponent->mRenderableLayer = RenderableLayer::LEVEL_FLOOR_LEVEL;

    auto levelResidentComponent = std::make_unique<LevelResidentComponent>();
    levelResidentComponent->mLevelNameId = levelNameId;

    world.AddComponent<TransformComponent>(modelEntityId, std::move(transformComponent));
    world.AddComponent<LevelResidentComponent>(modelEntityId, std::move(levelResidentComponent));
    world.AddComponent<RenderableComponent>(modelEntityId, std::move(renderableComponent));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
