//
//  PokedexUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/08/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokedexUtils.h"
#include "PokemonUtils.h"
#include "../components/PokedexStateSingletonComponent.h"
#include "../components/PokemonBaseStatsSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"

#include <algorithm>

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

PokedexEntryType GetPokedexEntryTypeForPokemon
(
    const StringId pokemonName,
    const ecs::World& world
)
{
    const auto& baseStats = GetPokemonBaseStats(pokemonName, world);
    const auto& pokedexStateComponent = world.GetSingletonComponent<PokedexStateSingletonComponent>();

    return pokedexStateComponent.mPokedexEntries[baseStats.mId - 1];
}

std::string GetPokedexCompletionRatingText
(
    const ecs::World& world
)
{    
    const auto ownedPokemon = GetNumberOfPokemonWithPokedexEntryType(PokedexEntryType::OWNED, world);
    const auto seenPokemon  = ownedPokemon + GetNumberOfPokemonWithPokedexEntryType(PokedexEntryType::SEEN, world);

    std::string pokedexCompletionText = "POK^DEX comp-#letion is:#@";
    pokedexCompletionText += std::to_string(seenPokemon) + " POK^MON seen#";
    pokedexCompletionText += std::to_string(ownedPokemon) + " POK^MON owned#@";
    pokedexCompletionText += "PROF.OAK's#Rating:#@";

    //TODO: Populate with other dialogs
    pokedexCompletionText += "You still have#lots to do.#Look for POK^MON#in grassy areas!";

    return pokedexCompletionText;
}

StringId GetPokemonNameFromPokedexId
(
    const int pokemonPokedexId,
    const ecs::World& world
)
{
    const auto& baseStatsComponent = world.GetSingletonComponent<PokemonBaseStatsSingletonComponent>();
    return std::find_if
    (
        baseStatsComponent.mPokemonBaseStats.cbegin(),
        baseStatsComponent.mPokemonBaseStats.cend(),
        [pokemonPokedexId](const std::pair<const StringId, PokemonBaseStats>& entry) 
        {
            return entry.second.mId == pokemonPokedexId;
        }
    )->first;
}

void ChangePokedexEntryForPokemon
(
    const StringId pokemonName,
    const PokedexEntryType pokedexEntryType,
    const ecs::World& world
)
{
    const auto& baseStats = GetPokemonBaseStats(pokemonName, world);
    auto& pokedexStateComponent = world.GetSingletonComponent<PokedexStateSingletonComponent>();
    pokedexStateComponent.mPokedexEntries[baseStats.mId - 1] = pokedexEntryType;
}

int GetNumberOfPokemonWithPokedexEntryType
(
    const PokedexEntryType pokedexEntryType,
    const ecs::World& world
)
{
    const auto& pokedexStateComponent = world.GetSingletonComponent<PokedexStateSingletonComponent>();
    return std::count(pokedexStateComponent.mPokedexEntries.cbegin(), pokedexStateComponent.mPokedexEntries.cend(), pokedexEntryType);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

