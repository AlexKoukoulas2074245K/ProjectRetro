//
//  DarkenedOpponentsIntroEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "DarkenedOpponentsIntroEncounterFlowState.h"
#include "OpponentIntroTextEncounterFlowState.h"
#include "../../ECS.h"
#include "../../common/components/TransformComponent.h"
#include "../../overworld/components/TransitionAnimationStateSingletonComponent.h"
#include "../../resources/MeshUtils.h"
#include "../../rendering/components/RenderableComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

DarkenedOpponentsIntroEncounterFlowState::DarkenedOpponentsIntroEncounterFlowState(ecs::World& world)
    : BaseEncounterFlowState(world)
{    
    const auto playerTrainerSpriteEntity = world.CreateEntity();
    
    auto renderableComponent                    = std::make_unique<RenderableComponent>();
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + "trainers.png");
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mRenderableLayer       = RenderableLayer::TOP;
    renderableComponent->mAffectedByPerspective = false;
    
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations
    (
        6,
        4,
        10,
        5,
        false,
        "camera_facing_quad",
        renderableComponent->mActiveAnimationNameId,
        *renderableComponent
    );
    
    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = glm::vec3(-0.37f, 0.05f, 0.0f);
    transformComponent->mScale    = glm::vec3(0.5f, 0.5f, 1.0f);

    mWorld.AddComponent<RenderableComponent>(playerTrainerSpriteEntity, std::move(renderableComponent));
    mWorld.AddComponent<TransformComponent>(playerTrainerSpriteEntity, std::move(transformComponent));
    
    const auto enemyTrainerSpriteEntity = world.CreateEntity();
    
    renderableComponent                         = std::make_unique<RenderableComponent>();
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + "trainers.png");
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mRenderableLayer       = RenderableLayer::TOP;
    renderableComponent->mAffectedByPerspective = false;
    
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations
    (
        4,
        4,
        10,
        5,
        false,
        "camera_facing_quad",
        renderableComponent->mActiveAnimationNameId,
        *renderableComponent
    );
    
    transformComponent            = std::make_unique<TransformComponent>();
    transformComponent->mPosition = glm::vec3(0.37f, 0.56f, 0.0f);
    transformComponent->mScale    = glm::vec3(0.5f, 0.5f, 1.0f);
    
    mWorld.AddComponent<RenderableComponent>(enemyTrainerSpriteEntity, std::move(renderableComponent));
    mWorld.AddComponent<TransformComponent>(enemyTrainerSpriteEntity, std::move(transformComponent));
    
    mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>().mAnimationProgressionStep = 2;
}

void DarkenedOpponentsIntroEncounterFlowState::VUpdate(const float)
{
    CompleteAndTransitionTo<OpponentIntroTextEncounterFlowState>();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
