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
#include "OverworldUtils.h"
#include "OverworldCharacterLoadingUtils.h"
#include "../components/AnimatedFlowerTagComponent.h"
#include "../components/LevelResidentComponent.h"
#include "../components/NpcAiComponent.h"
#include "../components/MovementStateComponent.h"
#include "../components/SeaTileTagComponent.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/utils/OSMessageBox.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/utils/Colors.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <json.hpp>
#include <unordered_map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string LEVEL_GROUND_LAYER_MODEL_FILE_NAME    = "2d_out_empty_floor.obj";
static const std::string LEVEL_GROUND_LAYER_TEXTURE_NAME_TRAIL = "_groundLayer.png";
static const std::string LEVEL_SEA_TILE_MODEL_NAME             = "sea_tile";

static const float ANIMATED_FLOWER_SCALE = 0.7f;

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
    ecs::World& world,
    TileCoords& minSeaTileCoords,
    TileCoords& maxSeaTileCoords
);

static void SetTileTrait
(
    const nlohmann::basic_json<>& tileTraitEntryJsonObject,
    LevelTilemap& levelTilemap
);

static void PadExtraSeaTiles
(
    const StringId levelNameId,
    const int levelTilemapCols,
    const int levelTilemapRows,
    const TileCoords& minSeaTileCoords,
    const TileCoords& maxSeaTileCoords,
    ecs::World& world
);

static void CheckAndLoadEncounterInfo
(
    LevelModelComponent& levelModelComponent
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId LoadAndCreateLevelByName(const StringId levelName, ecs::World& world)
{
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    
    // Get level data file resource
    const auto levelFilePath = ResourceLoadingService::RES_LEVELS_ROOT + levelName.GetString() + ".json";
    resourceLoadingService.LoadResource(levelFilePath);
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
    levelModelComponent->mLevelTilemap      = CreateTilemapWithDimensions(levelTilemapCols, levelTilemapRows);
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
    TileCoords minSeaTileCoords(levelTilemapCols, levelTilemapRows);
    TileCoords maxSeaTileCoords(0, 0);
    for (const auto& modelEntry: levelJson["level_model_list"])
    {
        CreateLevelModelEntry(modelEntry, levelModelComponent->mLevelName, world, minSeaTileCoords, maxSeaTileCoords);
    }

    if 
    (
        minSeaTileCoords.mCol != levelTilemapCols ||
        minSeaTileCoords.mRow != levelTilemapRows ||
        maxSeaTileCoords.mCol != 0 ||
        maxSeaTileCoords.mRow != 0
    )
    {
        PadExtraSeaTiles(levelModelComponent->mLevelName, levelTilemapCols, levelTilemapRows, minSeaTileCoords, maxSeaTileCoords, world);
    }

    // Load tile traits
    for (const auto& tileTraitEntry: levelJson["level_tile_traits"])
    {
        SetTileTrait(tileTraitEntry, levelModelComponent->mLevelTilemap);
    }
    
    // Load encounter info if any
    CheckAndLoadEncounterInfo(*levelModelComponent);
    
    // Associate level component with entity
    world.AddComponent<LevelModelComponent>(levelEntityId, std::move(levelModelComponent));
    
    // Unload level file resource
    resourceLoadingService.UnloadResource(levelFilePath);
    
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
    transformComponent->mPosition.x += groundLayerX - GAME_TILE_SIZE / 2.0f;
    transformComponent->mPosition.y -= GAME_TILE_SIZE / 2.0f;
    transformComponent->mPosition.z += groundLayerZ - GAME_TILE_SIZE / 2.0f;

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
    const auto dialog       = npcEntryJsonObject["dialog"].get<std::string>();
    const auto hasSprite    = npcEntryJsonObject["direction"].get<int>() != -1;
    const auto direction    = static_cast<Direction>(math::Max(0, npcEntryJsonObject["direction"].get<int>()));
    const auto gameCol      = npcEntryJsonObject["game_col"].get<int>();
    const auto gameRow      = npcEntryJsonObject["game_row"].get<int>();
    const auto atlasCol     = npcEntryJsonObject["atlas_col"].get<int>();
    const auto atlasRow     = npcEntryJsonObject["atlas_row"].get<int>();
    
    const auto npcEntityId = world.CreateEntity();
    
    auto animationTimerComponent             = std::make_unique<AnimationTimerComponent>();
    animationTimerComponent->mAnimationTimer = std::make_unique<Timer>(movementType == CharacterMovementType::DYNAMIC ? CHARACTER_ANIMATION_FRAME_TIME : STATIONARY_NPC_RESET_TIME);
    animationTimerComponent->mAnimationTimer->Pause();
    
    auto directionComponent        = std::make_unique<DirectionComponent>();
    directionComponent->mDirection = direction;
    
    auto aiComponent            = std::make_unique<NpcAiComponent>();
    aiComponent->mMovementType  = movementType;
    aiComponent->mDialog        = dialog;
    aiComponent->mInitDirection = direction;
    
    auto levelResidentComponent          = std::make_unique<LevelResidentComponent>();
    levelResidentComponent->mLevelNameId = levelNameId;
    
    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = TileCoordsToPosition(gameCol, gameRow);
    
    auto movementStateComponent            = std::make_unique<MovementStateComponent>();
    movementStateComponent->mCurrentCoords = TileCoords(gameCol, gameRow);
    
    if (hasSprite)
    {
        auto renderableComponent = CreateRenderableComponentForSprite(CharacterSpriteData(movementType, atlasCol, atlasRow));
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(direction), *renderableComponent);

        world.AddComponent<RenderableComponent>(npcEntityId, std::move(renderableComponent));
    }    
    
    GetTile(gameCol, gameRow, levelTilemap).mTileOccupierEntityId = npcEntityId;
    GetTile(gameCol, gameRow, levelTilemap).mTileOccupierType     = TileOccupierType::NPC;
    
    world.AddComponent<AnimationTimerComponent>(npcEntityId, std::move(animationTimerComponent));
    world.AddComponent<TransformComponent>(npcEntityId, std::move(transformComponent));
    world.AddComponent<LevelResidentComponent>(npcEntityId, std::move(levelResidentComponent));
    world.AddComponent<NpcAiComponent>(npcEntityId, std::move(aiComponent));
    world.AddComponent<MovementStateComponent>(npcEntityId, std::move(movementStateComponent));
    world.AddComponent<DirectionComponent>(npcEntityId, std::move(directionComponent));    
}

void CreateLevelModelEntry
(
    const nlohmann::basic_json<>& modelEntryJsonObject, 
    const StringId levelNameId, 
    ecs::World& world,
    TileCoords& minSeaTileCoords,
    TileCoords& maxSeaTileCoords
)
{    
    static const std::vector<StringId> undergroundModels =
    {
        StringId("in_staircase_down")
    };
    
    static const std::vector<StringId> flowerModels =
    {
        StringId("out_flower_bottom_right"),
        StringId("out_flower_top_left"),
    };
    
    static const std::vector<StringId> grassModels =
    {
        StringId("out_grass_wild")
    };
    
    // Extract model name from the: 'model_name (col_dim, row_dim)' format
    const auto modelName             = StringSplit(modelEntryJsonObject["model_name"].get<std::string>(), ' ')[0];
    const auto isSeaTileModel        = StringStartsWith(modelName, LEVEL_SEA_TILE_MODEL_NAME);
    const auto isAnimatedFlowerModel = std::find(flowerModels.begin(), flowerModels.end(), modelName) != flowerModels.end();
    const auto isWildGrassModel      = std::find(grassModels.begin(), grassModels.end(), modelName) != grassModels.end();
    const auto isUndergroundModel    = std::find(undergroundModels.begin(), undergroundModels.end(), modelName) != undergroundModels.end();
    
    // In the case of sea tiles dont create any entities, since they will be populated at a future step
    if (isSeaTileModel)
    {        
        const auto modelGameCol = modelEntryJsonObject["game_col"].get<int>();
        const auto modelGameRow = modelEntryJsonObject["game_row"].get<int>();

        if (modelGameCol < minSeaTileCoords.mCol) minSeaTileCoords.mCol = modelGameCol;
        if (modelGameRow < minSeaTileCoords.mRow) minSeaTileCoords.mRow = modelGameRow;
        if (modelGameCol > maxSeaTileCoords.mCol) maxSeaTileCoords.mCol = modelGameCol;
        if (modelGameRow > maxSeaTileCoords.mRow) maxSeaTileCoords.mRow = modelGameRow;

        return;
    }
    
    const auto modelEntityId = world.CreateEntity();

    auto transformComponent         = std::make_unique<TransformComponent>();
    transformComponent->mPosition.x = modelEntryJsonObject["game_position_x"].get<float>();
    transformComponent->mPosition.y = 0.0f;
    transformComponent->mPosition.z = modelEntryJsonObject["game_position_z"].get<float>();

    std::unique_ptr<RenderableComponent> renderableComponent = nullptr;
    
    if (isAnimatedFlowerModel)
    {
        renderableComponent = CreateRenderableComponentForSprite(CharacterSpriteData(CharacterMovementType::STATIONARY, 0, 44));
        renderableComponent->mActiveAnimationNameId = SOUTH_ANIMATION_NAME_ID;
        
        if (StringEndsWith(modelName, "bottom_right"))
        {
            transformComponent->mPosition.x += GAME_TILE_SIZE/4;
            transformComponent->mPosition.z -= GAME_TILE_SIZE/4;
        }
        else
        {
            transformComponent->mPosition.x -= GAME_TILE_SIZE/4;
            transformComponent->mPosition.z += GAME_TILE_SIZE/4;
        }
        
        transformComponent->mPosition.y -= GAME_TILE_SIZE/4;
        transformComponent->mScale = glm::vec3(ANIMATED_FLOWER_SCALE, ANIMATED_FLOWER_SCALE, ANIMATED_FLOWER_SCALE);
        
        world.AddComponent<AnimatedFlowerTagComponent>(modelEntityId, std::make_unique<AnimatedFlowerTagComponent>());
    }
    else
    {
        renderableComponent = std::make_unique<RenderableComponent>();
        renderableComponent->mShaderNameId = StringId("basic");
        
        renderableComponent->mAnimationsToMeshes[StringId("default")].push_back
        (
            ResourceLoadingService::GetInstance().
            LoadResource(ResourceLoadingService::RES_MODELS_ROOT + modelName + ".obj"
        ));
        
        if (isUndergroundModel)
        {
            renderableComponent->mRenderableLayer = RenderableLayer::UNDERGROUND;
        }
        else if (isWildGrassModel)
        {
            renderableComponent->mRenderableLayer = RenderableLayer::UNDERGROUND;
        }
        else
        {
            renderableComponent->mRenderableLayer = RenderableLayer::LEVEL_FLOOR_LEVEL;
        }
        
        renderableComponent->mActiveAnimationNameId = StringId("default");
        renderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource
        (
            ResourceLoadingService::RES_TEXTURES_ROOT + modelName + ".png"
        );
    }
   
    auto levelResidentComponent          = std::make_unique<LevelResidentComponent>();
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
        { StringId("ENCOUNTER"),           TileTrait::ENCOUNTER },
        { StringId("SOLID"),               TileTrait::SOLID },
        { StringId("WARP"),                TileTrait::WARP },
        { StringId("NO_ANIM_WARP"),        TileTrait::NO_ANIM_WARP },
        { StringId("PRESS_WARP"),          TileTrait::PRESS_WARP },
        { StringId("JUMPING_LEDGE_BOT"),   TileTrait::JUMPING_LEDGE_BOT },
        { StringId("JUMPING_LEDGE_LEFT"),  TileTrait::JUMPING_LEDGE_LEFT },
        { StringId("JUMPING_LEDGE_RIGHT"), TileTrait::JUMPING_LEDGE_RIGHT },
    };
    
    const auto gameCol   = tileTraitEntryJsonObject["game_col"].get<int>();
    const auto gameRow   = tileTraitEntryJsonObject["game_row"].get<int>();
    const auto traitName = tileTraitEntryJsonObject["tile_traits"].get<std::string>();
    
    GetTile(gameCol, gameRow, levelTilemap).mTileTrait = traitNamesToTraitEnums.at(traitName);
}

static void PadExtraSeaTiles
(
    const StringId levelNameId,
    const int levelTilemapCols,
    const int levelTilemapRows,
    const TileCoords& minSeaTileCoords,
    const TileCoords& maxSeaTileCoords,
    ecs::World& world
)
{
    for (auto row = math::Max(0, minSeaTileCoords.mRow - 1); row <= math::Min(levelTilemapRows - 1, maxSeaTileCoords.mRow + 1); ++row)
    {
        for (auto col = math::Max(0, minSeaTileCoords.mCol - 1); col <= math::Min(levelTilemapCols - 1, maxSeaTileCoords.mCol + 1); ++col)
        {
            const auto modelEntityId = world.CreateEntity();

            auto transformComponent = std::make_unique<TransformComponent>();
            transformComponent->mPosition.x = col * GAME_TILE_SIZE;
            transformComponent->mPosition.y = -0.05f;
            transformComponent->mPosition.z = row * GAME_TILE_SIZE;

            auto renderableComponent = std::make_unique<RenderableComponent>();
            renderableComponent->mShaderNameId = StringId("basic");
            renderableComponent->mAnimationsToMeshes[StringId("default")].push_back
            (
                ResourceLoadingService::GetInstance().
                LoadResource(ResourceLoadingService::RES_MODELS_ROOT + LEVEL_SEA_TILE_MODEL_NAME + ".obj"
            ));            

            renderableComponent->mRenderableLayer = RenderableLayer::UNDERGROUND;
            renderableComponent->mActiveAnimationNameId = StringId("default");
            renderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource
            (
                ResourceLoadingService::RES_TEXTURES_ROOT + LEVEL_SEA_TILE_MODEL_NAME + ".png"
            );

            auto levelResidentComponent = std::make_unique<LevelResidentComponent>();
            levelResidentComponent->mLevelNameId = levelNameId;

            world.AddComponent<SeaTileTagComponent>(modelEntityId, std::make_unique<SeaTileTagComponent>());
            world.AddComponent<TransformComponent>(modelEntityId, std::move(transformComponent));
            world.AddComponent<LevelResidentComponent>(modelEntityId, std::move(levelResidentComponent));
            world.AddComponent<RenderableComponent>(modelEntityId, std::move(renderableComponent));
        }
    }    
}

void CheckAndLoadEncounterInfo
(
    LevelModelComponent& levelModelComponent
)
{
    const auto encounterDataFilePath = ResourceLoadingService::ENCOUNTER_DATA_ROOT + levelModelComponent.mLevelName.GetString() + ".json";
    auto& resourceLoadingService     = ResourceLoadingService::GetInstance();
    
    // Check whether this level has encounter data associated with it
    if (resourceLoadingService.DoesResourceExist(encounterDataFilePath) == false)
    {
        return;
    }
    
    // Check in case the resource has already been loaded
    if (resourceLoadingService.HasLoadedResource(encounterDataFilePath) == false)
    {
        resourceLoadingService.LoadResource(encounterDataFilePath);
    }
    
    // Get the data file handle
    const auto& encounterDataFileResource = resourceLoadingService.GetResource<DataFileResource>(encounterDataFilePath);
    
    // Create encounter json object
    const auto encounterJson = nlohmann::json::parse(encounterDataFileResource.GetContents());
    
    // Extract encounter density
    levelModelComponent.mEncounterDensity = encounterJson["density"].get<int>();
    
    // Extract encounters
    for (const auto& encounterInfoJsonObject: encounterJson["encounters"])
    {
        WildEncounterInfo encounterInfo;
        encounterInfo.mRate                   = encounterInfoJsonObject["rate"].get<int>();
        encounterInfo.mPokemonInfo.mName      = StringId(encounterInfoJsonObject["pokemon"].get<std::string>());
        encounterInfo.mPokemonInfo.mLevel     = encounterInfoJsonObject["level"].get<int>();

        //TODO: Replace with correct pokedex id
        encounterInfo.mPokemonInfo.mPokedexId = 1;
        
        levelModelComponent.mWildEncounters.push_back(encounterInfo);
    }
    
    resourceLoadingService.UnloadResource(encounterDataFilePath);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
