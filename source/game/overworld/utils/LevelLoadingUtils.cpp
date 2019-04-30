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
#include "OverworldCharacterLoadingUtils.h"
#include "../components/LevelResidentComponent.h"
#include "../../common/utils/MessageBox.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/utils/Colors.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <json.hpp>
#include <unordered_map>

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

static void CreateNpc
(
    const nlohmann::basic_json<>& npcEntryJsonObject,
    const StringId levelNameId,
    LevelTilemap& levelTilemap,
    ecs::World& world
);

static void CreateLevelModelEntry
(
    const nlohmann::basic_json<>& modelEntryJsonObject, 
    const StringId levelNameId, 
    ecs::World& world
);

static void SetTileTrait
(
    const nlohmann::basic_json<>& tileTraitEntryJsonObject,
    LevelTilemap& levelTilemap
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
    const auto levelEntityId = world.CreateEntity();
    auto levelModelComponent = std::make_unique<LevelModelComponent>();

    levelModelComponent->mLevelName         = StringId(levelName);
    levelModelComponent->mLevelTilemap      = InitializeTilemapWithDimensions(levelTilemapCols, levelTilemapRows);
    levelModelComponent->mCols              = levelTilemapCols;
    levelModelComponent->mRows              = levelTilemapRows;
    levelModelComponent->mGroundLayerEntity = world.CreateEntity();

    // Extract level palette color
    const auto hasPaletteEntry = levelJson["level_header"].count("color") != 0;

    if (hasPaletteEntry)
    {
        static const std::unordered_map<StringId, glm::vec4, StringIdHasher> levelColorNamesToValues =
        {
            { StringId("PALLET"),    colors::PALLET_COLOR },
            { StringId("VIRIDIAN"),  colors::VIRIDIAN_COLOR },
            { StringId("PEWTER"),    colors::PEWTER_COLOR },
            { StringId("CAVE"),      colors::CAVE_COLOR },
            { StringId("CERULEAN"),  colors::CERULEAN_COLOR },
            { StringId("VERMILION"), colors::VERMILION_COLOR },
            { StringId("LAVENDER"),  colors::LAVENDER_COLOR }
        };

        levelModelComponent->mLevelColor = levelColorNamesToValues.at(StringId(levelJson["level_header"]["color"].get<std::string>()));
    }
    else
    {
        ShowMessageBox
        (
            MessageBoxType::WARNING,
            "Palette Info Missing",
            "No palette info was found in level: " + levelName.GetString()
        );
    }

    // Create optimized ground layer texture for level
    const auto groundLayerTextureName = levelJson["level_ground_layer_game"][0]["texture_name"];
    const auto groundLayerPositionX   = levelJson["level_ground_layer_game"][0]["game_position_x"];
    const auto groundLayerPositionZ   = levelJson["level_ground_layer_game"][0]["game_position_z"];

    CreateLevelGroundLayer
    (
        levelModelComponent->mLevelName,
        levelTilemapCols,
        levelTilemapRows,
        groundLayerPositionX,
        groundLayerPositionZ,
        levelModelComponent->mGroundLayerEntity,
        world
    );
    
    // Load NPC list
    for (const auto& npcJsonEntry: levelJson["level_npc_list"])
    {
        CreateNpc(npcJsonEntry, levelModelComponent->mLevelName, levelModelComponent->mLevelTilemap, world);
    }

    // Load model list
    for (const auto& modelEntry: levelJson["level_model_list"])
    {
        CreateLevelModelEntry(modelEntry, levelModelComponent->mLevelName, world);
    }

    // Load tile traits
    for (const auto& tileTraitEntry: levelJson["level_tile_traits"])
    {
        SetTileTrait(tileTraitEntry, levelModelComponent->mLevelTilemap);
    }
    
    world.AddComponent<LevelModelComponent>(levelEntityId, std::move(levelModelComponent));
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

static void CreateNpc
(
    const nlohmann::basic_json<>& npcEntryJsonObject,
    const StringId levelNameId,
    LevelTilemap& levelTilemap,
    ecs::World& world
)
{
    static const std::unordered_map<StringId, CharacterMovementType, StringIdHasher> characterMovementTypesNamesToEnums =
    {
        { StringId("STATIC"),     CharacterMovementType::STATIC },
        { StringId("STATIONARY"), CharacterMovementType::STATIONARY },
        { StringId("DYNAMIC"),    CharacterMovementType::DYNAMIC }
    };
    
    const auto movementType = characterMovementTypesNamesToEnums.at(StringId(npcEntryJsonObject["movement_type"]));
    const auto gameCol      = npcEntryJsonObject["game_col"].get<int>();
    const auto gameRow      = npcEntryJsonObject["game_row"].get<int>();
    const auto atlasCol     = npcEntryJsonObject["atlas_col"].get<int>();
    const auto atlasRow     = npcEntryJsonObject["atlas_row"].get<int>();
    
    const auto npcEntityId = world.CreateEntity();
    
    auto levelResidentComponent = std::make_unique<LevelResidentComponent>();
    levelResidentComponent->mLevelNameId = levelNameId;
    
    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = TileCoordsToPosition(gameCol, gameRow);
    
    GetTile(gameCol, gameRow, levelTilemap).mTileOccupierEntityId = npcEntityId;
    GetTile(gameCol, gameRow, levelTilemap).mTileOccupierType     = TileOccupierType::NPC;
    
    world.AddComponent<TransformComponent>(npcEntityId, std::move(transformComponent));
    world.AddComponent<LevelResidentComponent>(npcEntityId, std::move(levelResidentComponent));
    world.AddComponent<RenderableComponent>(npcEntityId, CreateRenderableComponentForSprite(CharacterSpriteData(movementType, atlasCol, atlasRow)));
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

    auto levelResidentComponent = std::make_unique<LevelResidentComponent>();
    levelResidentComponent->mLevelNameId = levelNameId;

    world.AddComponent<TransformComponent>(modelEntityId, std::move(transformComponent));
    world.AddComponent<LevelResidentComponent>(modelEntityId, std::move(levelResidentComponent));
    world.AddComponent<RenderableComponent>(modelEntityId, std::move(renderableComponent));
}

void SetTileTrait
(
    const nlohmann::basic_json<>& tileTraitEntryJsonObject,
    LevelTilemap& levelTilemap
)
{
    static const std::unordered_map<StringId, TileTrait, StringIdHasher> traitNamesToTraitEnums =
    {
        { StringId("ENCOUNTER"),    TileTrait::ENCOUNTER },
        { StringId("SOLID"),        TileTrait::SOLID },
        { StringId("WARP"),         TileTrait::WARP },
        { StringId("NO_ANIM_WARP"), TileTrait::NO_ANIM_WARP },
        { StringId("PRESS_WARP"),   TileTrait::PRESS_WARP },
    };
    
    const auto gameCol   = tileTraitEntryJsonObject["game_col"].get<int>();
    const auto gameRow   = tileTraitEntryJsonObject["game_row"].get<int>();
    const auto traitName = tileTraitEntryJsonObject["tile_traits"].get<std::string>();
    
    GetTile(gameCol, gameRow, levelTilemap).mTileTrait = traitNamesToTraitEnums.at(traitName);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
