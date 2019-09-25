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

static const glm::vec3 TOWN_MAP_BACKGROUND_POSITION = glm::vec3(0.0f, 0.0f, 0.1f);
static const glm::vec3 TOWN_MAP_BACKGROUND_SCALE    = glm::vec3(2.0f, 2.0f, 1.0f);

static const std::string TOWN_MAP_SPRITE_MODEL_FILE_NAME       = "town_map_sprite.obj";
static const std::string TOWN_MAP_BACKGROUND_TEXTURE_FILE_NAME = "town_map_background.png";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId LoadAndCreateTownMapBackground(ecs::World& world)
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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
