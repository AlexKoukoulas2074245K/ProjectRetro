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
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/MathUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 TOWN_MAP_BACKGROUND_COVER_POSITION = glm::vec3(0.0f, 0.0f, 0.01f);
const glm::vec3 TOWN_MAP_BACKGROUND_POSITION       = glm::vec3(0.0f, 0.0f, -0.1f);
const glm::vec3 TOWN_MAP_BACKGROUND_COVER_SCALE    = glm::vec3(2.0f, 2.0f, 2.0f);
const glm::vec3 TOWN_MAP_BACKGROUND_SCALE          = glm::vec3(1.0f, 1.0f, 1.0f);

const std::string TOWN_MAP_SPRITE_MODEL_FILE_NAME             = "camera_facing_quad_hud.obj";
const std::string TOWN_MAP_BACKGROUND_COVER_TEXTURE_FILE_NAME = "town_map_background_cover.png";
const std::string TOWN_MAP_BACKGROUND_TEXTURE_FILE_NAME       = "town_map_background.png";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId LoadAndCreateTownMapBackgroundCover(ecs::World& world)
{
    const auto backgroundCoverEntityId = world.CreateEntity();

    auto renderableComponent = std::make_unique<RenderableComponent>();

    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + TOWN_MAP_BACKGROUND_COVER_TEXTURE_FILE_NAME;
    renderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;

    const auto modelPath = ResourceLoadingService::RES_MODELS_ROOT + TOWN_MAP_SPRITE_MODEL_FILE_NAME;
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = TOWN_MAP_BACKGROUND_COVER_POSITION;
    transformComponent->mScale = TOWN_MAP_BACKGROUND_COVER_SCALE;

    world.AddComponent<RenderableComponent>(backgroundCoverEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(backgroundCoverEntityId, std::move(transformComponent));

    return backgroundCoverEntityId;
}

ecs::EntityId LoadAndCreateTownMapBackground(ecs::World& world)
{
    const auto backgroundEntityId = world.CreateEntity();

    auto renderableComponent = std::make_unique<RenderableComponent>();

    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + TOWN_MAP_BACKGROUND_TEXTURE_FILE_NAME;
    renderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;

    const auto modelPath = ResourceLoadingService::RES_MODELS_ROOT + TOWN_MAP_SPRITE_MODEL_FILE_NAME;
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = TOWN_MAP_BACKGROUND_POSITION;
    transformComponent->mScale = TOWN_MAP_BACKGROUND_SCALE;

    world.AddComponent<RenderableComponent>(backgroundEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(backgroundEntityId, std::move(transformComponent));

    return backgroundEntityId;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
