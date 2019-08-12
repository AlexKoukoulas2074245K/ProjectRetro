//
//  PokedexSpriteUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/08/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokedexSpriteUtils.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../components/TransformComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string POKEDEX_ENTRY_DATA_SCREEN_TEXTURE_FILE_NAME = "pokedex_data_screen.png";
static const std::string POKEDEX_ENTRY_DATA_SCREEN_MODEL_FILE_NAME = "pokemon_stats_display_quad.obj";

static const glm::vec3 POKEDEX_ENTRY_DATA_SCREEN_POSITION = glm::vec3(0.0f, 0.0f, -0.1f);
static const glm::vec3 POKEDEX_ENTRY_DATA_SCREEN_SCALE    = glm::vec3(2.2f, 2.2f, 1.0f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId LoadAndCreatePokedexPokemonDataScreen
(
    ecs::World& world
)
{
    const auto pokedexPokemonEntryDataScreenEntityId = world.CreateEntity();

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    auto renderableComponent                    = std::make_unique<RenderableComponent>();
    renderableComponent->mTextureResourceId     = resourceLoadingService.LoadResource(ResourceLoadingService::RES_TEXTURES_ROOT + POKEDEX_ENTRY_DATA_SCREEN_TEXTURE_FILE_NAME);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(ResourceLoadingService::RES_MODELS_ROOT + POKEDEX_ENTRY_DATA_SCREEN_MODEL_FILE_NAME));

    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = POKEDEX_ENTRY_DATA_SCREEN_POSITION;
    transformComponent->mScale    = POKEDEX_ENTRY_DATA_SCREEN_SCALE;

    world.AddComponent<RenderableComponent>(pokedexPokemonEntryDataScreenEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(pokedexPokemonEntryDataScreenEntityId, std::move(transformComponent));

    return pokedexPokemonEntryDataScreenEntityId;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

