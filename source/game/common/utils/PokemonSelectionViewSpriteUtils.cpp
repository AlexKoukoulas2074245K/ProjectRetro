//
//  PokemonSelectionViewSpriteUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/07/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonSelectionViewSpriteUtils.h"
#include "TextboxUtils.h"
#include "MathUtils.h"
#include "../components/CursorComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../components/TransformComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string POKEMON_STATS_DISPLAY_SCREEN_1_TEXTURE_FILE_NAME    = "pokemon_selection_vew_stats_display_screen_1.png";
static const std::string POKEMON_STATS_DISPLAY_SCREEN_2_TEXTURE_FILE_NAME    = "pokemon_selection_vew_stats_display_screen_2.png";
static const std::string POKEMON_STATS_DISPLAY_SCREEN_MODEL_FILE_NAME        = "pokemon_stats_display_quad.obj";
static const std::string POKEMON_SELECTION_VIEW_BACKGROUND_MODEL_FILE_NAME   = "pokemon_selection_view_sprite.obj";
static const std::string POKEMON_SELECTION_VIEW_BACKGROUND_TEXTURE_FILE_NAME = "pokemon_selection_view_background.png";

static const glm::vec3 POKEMON_SELECTED_TEXTBOX_POSITION = glm::vec3(0.378f, -0.615f, -0.4f);
static const glm::vec3 POKEMON_STATS_DISPLAY_POSITION    = glm::vec3(0.0f, 0.0f, -0.6f);
static const glm::vec3 POKEMON_STATS_DISPLAY_SCALE       = glm::vec3(2.2f, 2.2f, 1.0f);

static const int POKEMON_SELECTED_TEXTBOX_COLS = 9;
static const int POKEMON_SELECTED_TEXTBOX_ROWS = 7;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId CreatePokemonSelectionViewSelectionTextbox
(
    bool isInEncounter,
    ecs::World& world
)
{
    const auto textboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        POKEMON_SELECTED_TEXTBOX_COLS,
        POKEMON_SELECTED_TEXTBOX_ROWS,
        POKEMON_SELECTED_TEXTBOX_POSITION.x,
        POKEMON_SELECTED_TEXTBOX_POSITION.y,
        POKEMON_SELECTED_TEXTBOX_POSITION.z,
        world
    );

    if (isInEncounter)
    {
        WriteTextAtTextboxCoords(textboxEntityId, "SWITCH", 2, 1, world);
        WriteTextAtTextboxCoords(textboxEntityId, "STATS", 2, 3, world);
    }
    else
    {
        WriteTextAtTextboxCoords(textboxEntityId, "STATS", 2, 1, world);
        WriteTextAtTextboxCoords(textboxEntityId, "SWITCH", 2, 3, world);
    }
    WriteTextAtTextboxCoords(textboxEntityId, "CANCEL", 2, 5, world);

    auto cursorComponent = std::make_unique<CursorComponent>();    

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = 0;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = 3;

    cursorComponent->mCursorDisplayHorizontalTileOffset     = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 1;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;

    WriteCharAtTextboxCoords
    (
        textboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;
    
    world.AddComponent<CursorComponent>(textboxEntityId, std::move(cursorComponent));

    return textboxEntityId;
}

ecs::EntityId LoadAndCreatePokemonSelectionViewBackgroundCover
(
    const glm::vec3& position,
    const glm::vec3& scale,
    ecs::World& world
)
{
    const auto backgroundEntityId = world.CreateEntity();

    auto renderableComponent = std::make_unique<RenderableComponent>();

    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + POKEMON_SELECTION_VIEW_BACKGROUND_TEXTURE_FILE_NAME;
    renderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;

    const auto modelPath = ResourceLoadingService::RES_MODELS_ROOT + POKEMON_SELECTION_VIEW_BACKGROUND_MODEL_FILE_NAME;
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = position;
    transformComponent->mScale    = scale;

    world.AddComponent<RenderableComponent>(backgroundEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(backgroundEntityId, std::move(transformComponent));

    return backgroundEntityId;
}

ecs::EntityId LoadAndCreatePokemonStatsDisplayScreen
(
    bool isScreen1,
    ecs::World& world
)
{
    const auto pokemonStatsDisplayScreen1 = world.CreateEntity();

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    auto renderableComponent = std::make_unique<RenderableComponent>();
    renderableComponent->mTextureResourceId     = resourceLoadingService.LoadResource(ResourceLoadingService::RES_TEXTURES_ROOT + (isScreen1 ? POKEMON_STATS_DISPLAY_SCREEN_1_TEXTURE_FILE_NAME : POKEMON_STATS_DISPLAY_SCREEN_2_TEXTURE_FILE_NAME));
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;    
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(ResourceLoadingService::RES_MODELS_ROOT + POKEMON_STATS_DISPLAY_SCREEN_MODEL_FILE_NAME));
    
    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = POKEMON_STATS_DISPLAY_POSITION;
    transformComponent->mScale    = POKEMON_STATS_DISPLAY_SCALE;
       
    world.AddComponent<RenderableComponent>(pokemonStatsDisplayScreen1, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(pokemonStatsDisplayScreen1, std::move(transformComponent));

    return pokemonStatsDisplayScreen1;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

