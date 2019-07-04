//
//  EncounterSpriteUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "EncounterSpriteUtils.h"
#include "../../common/GameConstants.h"
#include "../../common/components/TransformComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/MeshUtils.h"
#include "../../resources/ResourceLoadingService.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string ENCOUNTER_SPRITE_MODEL_NAME                  = "camera_facing_quad";
static const std::string POKEMON_BATTLE_SPRITE_MODEL_NAME             = "pokemon_battle_sprite";
static const std::string ENCOUNTER_SPRITE_ANIMATION_NAME              = "default";
static const std::string ENCOUNTER_SPRITE_SHADER_NAME                 = "gui";
static const std::string TRAINER_ATLAS_FILE_NAME                      = "trainers.png";
static const std::string PLAYER_ROSTER_DISPLAY_TEXTURE_NAME           = "battle_player_roster";
static const std::string OPPONENT_POKEMON_STATUS_DISPLAY_TEXTURE_NAME = "battle_enemy_pokemon_status";
static const std::string ENCOUNTER_EDGE_TEXTURE_NAME                  = "battle_edge";

static const glm::vec3 ENCOUNTER_LEFT_EDGE_POSITION  = glm::vec3(-0.937f, 0.0f, -1.0f);
static const glm::vec3 ENCOUNTER_RIGHT_EDGE_POSITION = glm::vec3(0.937f, 0.0f, -1.0f);
static const glm::vec3 ENCOUNTER_EDGE_SCALE          = glm::vec3(0.5f, 1.5f, 1.0f);

static const int TRAINER_ATLAS_COLS = 10;
static const int TRAINER_ATLAS_ROWS = 5;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId LoadAndCreateTrainerSprite
(
    const int atlasCol,
    const int atlasRow,
    const glm::vec3& spritePosition,
    const glm::vec3& spriteScale,
    ecs::World& world
)
{
    const auto trainerSpriteEntityId = world.CreateEntity();
    
    auto renderableComponent                    = std::make_unique<RenderableComponent>();
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + TRAINER_ATLAS_FILE_NAME);
    renderableComponent->mActiveAnimationNameId = StringId(ENCOUNTER_SPRITE_ANIMATION_NAME);
    renderableComponent->mShaderNameId          = StringId(ENCOUNTER_SPRITE_SHADER_NAME);
    renderableComponent->mAffectedByPerspective = false;
    
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations
    (
        atlasCol,
        atlasRow,
        TRAINER_ATLAS_COLS,
        TRAINER_ATLAS_ROWS,
        false,
        ENCOUNTER_SPRITE_MODEL_NAME,
        renderableComponent->mActiveAnimationNameId,
        *renderableComponent
    );
    
    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = spritePosition;
    transformComponent->mScale    = spriteScale;
    
    world.AddComponent<RenderableComponent>(trainerSpriteEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(trainerSpriteEntityId, std::move(transformComponent));
    
    return trainerSpriteEntityId;
}

ecs::EntityId LoadAndCreatePokemonSprite
(
    const StringId pokemonName,
    const bool frontFace,
    const glm::vec3& spritePosition,
    const glm::vec3& spriteScale,
    ecs::World& world
)
{
    const auto pokemonSpriteEntityId = world.CreateEntity();
    
    auto renderableComponent                    = std::make_unique<RenderableComponent>();
    
    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + (frontFace ? "pkmnfront/" : "pkmnback/") + pokemonName.GetString() + ".png";
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId(ENCOUNTER_SPRITE_ANIMATION_NAME);
    renderableComponent->mShaderNameId          = StringId(ENCOUNTER_SPRITE_SHADER_NAME);
    renderableComponent->mAffectedByPerspective = false;
    
    const auto modelPath         = ResourceLoadingService::RES_MODELS_ROOT + POKEMON_BATTLE_SPRITE_MODEL_NAME + ".obj";
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    renderableComponent->mAnimationsToMeshes[StringId(ENCOUNTER_SPRITE_ANIMATION_NAME)].push_back(resourceLoadingService.LoadResource(modelPath));
    
    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = spritePosition;
    transformComponent->mScale    = spriteScale;
    
    world.AddComponent<RenderableComponent>(pokemonSpriteEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(pokemonSpriteEntityId, std::move(transformComponent));
    
    return pokemonSpriteEntityId;
}

ecs::EntityId LoadAndCreatePlayerRosterDisplay
(
    const glm::vec3& spritePosition,
    const glm::vec3& spriteScale,
    ecs::World& world
)
{
    const auto playerRosterDisplayEntityId = world.CreateEntity();
    
    auto renderableComponent = std::make_unique<RenderableComponent>();
    
    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + PLAYER_ROSTER_DISPLAY_TEXTURE_NAME + ".png";
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;
    
    const auto modelPath         = ResourceLoadingService::RES_MODELS_ROOT + POKEMON_BATTLE_SPRITE_MODEL_NAME + ".obj";
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));
    
    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = spritePosition;
    transformComponent->mScale    = spriteScale;
    
    world.AddComponent<RenderableComponent>(playerRosterDisplayEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(playerRosterDisplayEntityId, std::move(transformComponent));
    
    return playerRosterDisplayEntityId;
}

ecs::EntityId LoadAndCreateOpponentPokemonStatusDisplay
(
    PokemonInfo&,
    const glm::vec3& spritePosition,
    const glm::vec3& spriteScale,
    ecs::World& world
)
{
    const auto opponentStatusDisplayEntityId = world.CreateEntity();
    
    auto renderableComponent = std::make_unique<RenderableComponent>();
    
    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + OPPONENT_POKEMON_STATUS_DISPLAY_TEXTURE_NAME + ".png";
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;
    
    const auto modelPath         = ResourceLoadingService::RES_MODELS_ROOT + POKEMON_BATTLE_SPRITE_MODEL_NAME + ".obj";
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));
    
    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = spritePosition;
    transformComponent->mScale    = spriteScale;
    
    world.AddComponent<RenderableComponent>(opponentStatusDisplayEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(opponentStatusDisplayEntityId, std::move(transformComponent));
    
    return opponentStatusDisplayEntityId;
}

std::pair<ecs::EntityId, ecs::EntityId> LoadAndCreateLevelEdges(ecs::World& world)
{
    const auto ledgeEdgeEntityId = world.CreateEntity();

    auto renderableComponent = std::make_unique<RenderableComponent>();

    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + ENCOUNTER_EDGE_TEXTURE_NAME + ".png";
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;

    const auto modelPath         = ResourceLoadingService::RES_MODELS_ROOT + POKEMON_BATTLE_SPRITE_MODEL_NAME + ".obj";
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = ENCOUNTER_LEFT_EDGE_POSITION;
    transformComponent->mScale    = ENCOUNTER_EDGE_SCALE;

    world.AddComponent<RenderableComponent>(ledgeEdgeEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(ledgeEdgeEntityId, std::move(transformComponent));

    const auto rightEdgeEntityId = world.CreateEntity();

    renderableComponent = std::make_unique<RenderableComponent>();
    
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    transformComponent            = std::make_unique<TransformComponent>();
    transformComponent->mPosition = ENCOUNTER_RIGHT_EDGE_POSITION;
    transformComponent->mScale    = ENCOUNTER_EDGE_SCALE;

    world.AddComponent<RenderableComponent>(rightEdgeEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(rightEdgeEntityId, std::move(transformComponent));


    return std::make_pair(ledgeEdgeEntityId, rightEdgeEntityId);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
