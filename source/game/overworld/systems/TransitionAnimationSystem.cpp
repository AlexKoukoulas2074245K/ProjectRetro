//
//  TransitionAnimationSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TransitionAnimationSystem.h"
#include "../components/EncounterStateSingletonComponent.h"
#include "../components/TransitionAnimationStateSingletonComponent.h"
#include "../components/WarpConnectionsSingletonComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/utils/FileUtils.h"
#include "../../common/utils/Logging.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../resources/TextureResource.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float TransitionAnimationSystem::WARP_TRANSITION_STEP_DURATION      = 0.13f;
const float TransitionAnimationSystem::WILD_FLASH_ANIMATION_STEP_DURATION = 0.05f;
const float TransitionAnimationSystem::ENCOUNTER_ANIMATION_FRAME_DURATION = 0.05f;
const int TransitionAnimationSystem::TRANSITION_STEP_COUNT                = 3;
const int TransitionAnimationSystem::WILD_FLASH_CYCLE_REPEAT_COUNT        = 3;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TransitionAnimationSystem::TransitionAnimationSystem(ecs::World& world)
    : BaseSystem(world)
{
    mWorld.SetSingletonComponent<TransitionAnimationStateSingletonComponent>(std::make_unique<TransitionAnimationStateSingletonComponent>());
}

void TransitionAnimationSystem::VUpdateAssociatedComponents(const float dt) const
{    
    const auto& warpConnectionsComponent    = mWorld.GetSingletonComponent<WarpConnectionsSingletonComponent>();
    const auto& encounterStateComponent     = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();

    if (transitionAnimationStateComponent.mIsPlayingTransitionAnimation)
    {
        switch (transitionAnimationStateComponent.mTransitionAnimationType)
        {
            case TransitionAnimationType::WARP: UpdateWarpTransitionAnimation(dt); break;
            case TransitionAnimationType::WILD_FLASH: UpdateWildFlashTransitionAnimation(dt); break;
            case TransitionAnimationType::ENCOUNTER: UpdateEncounterTransitionAnimation(dt); break;
        }        
    }
    else if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
    {
        transitionAnimationStateComponent.mTransitionAnimationType       = TransitionAnimationType::WILD_FLASH;
        transitionAnimationStateComponent.mAnimationProgressionStep      = 0;
        transitionAnimationStateComponent.mAnimationCycleCompletionCount = 0;
        transitionAnimationStateComponent.mAscendingPalette              = true;
        transitionAnimationStateComponent.mIsPlayingTransitionAnimation  = true;
        transitionAnimationStateComponent.mAnimationTimer                = std::make_unique<Timer>(WILD_FLASH_ANIMATION_STEP_DURATION);
    }
    else if (warpConnectionsComponent.mHasPendingWarpConnection)
    {
        transitionAnimationStateComponent.mTransitionAnimationType      = TransitionAnimationType::WARP;
        transitionAnimationStateComponent.mIsPlayingTransitionAnimation = warpConnectionsComponent.mShouldPlayTransitionAnimation;
        transitionAnimationStateComponent.mAnimationProgressionStep     = 0;
        transitionAnimationStateComponent.mAnimationTimer               = std::make_unique<Timer>(WARP_TRANSITION_STEP_DURATION);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void TransitionAnimationSystem::UpdateWarpTransitionAnimation(const float dt) const
{
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();

    transitionAnimationStateComponent.mAnimationTimer->Update(dt);
    if (transitionAnimationStateComponent.mAnimationTimer->HasTicked())
    {
        transitionAnimationStateComponent.mAnimationTimer->Reset();
        if (++transitionAnimationStateComponent.mAnimationProgressionStep > TRANSITION_STEP_COUNT)
        {
            transitionAnimationStateComponent.mAnimationProgressionStep = 0;
            transitionAnimationStateComponent.mIsPlayingTransitionAnimation = false;
        }
    }
}

void TransitionAnimationSystem::UpdateWildFlashTransitionAnimation(const float dt) const
{
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    auto& encounterStateComponent           = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    transitionAnimationStateComponent.mAnimationTimer->Update(dt);
    if (transitionAnimationStateComponent.mAnimationTimer->HasTicked())
    {
        transitionAnimationStateComponent.mAnimationTimer->Reset();
        if (transitionAnimationStateComponent.mAscendingPalette)
        {
            if (++transitionAnimationStateComponent.mAnimationProgressionStep > TRANSITION_STEP_COUNT)
            {
                transitionAnimationStateComponent.mAscendingPalette = false;
            }
            
            if (transitionAnimationStateComponent.mAnimationProgressionStep == 0)
            {
                if (++transitionAnimationStateComponent.mAnimationCycleCompletionCount == WILD_FLASH_CYCLE_REPEAT_COUNT)
                {
                    encounterStateComponent.mEncounterState = EncounterState::ENCOUNTER_ANIMATION;
                    transitionAnimationStateComponent.mTransitionAnimationType = TransitionAnimationType::ENCOUNTER;
                    transitionAnimationStateComponent.mAnimationTimer = std::make_unique<Timer>(WILD_FLASH_ANIMATION_STEP_DURATION);
                    LoadEncounterSpecificAnimation();
                }
            }
        }
        else
        {
            if (--transitionAnimationStateComponent.mAnimationProgressionStep < -TRANSITION_STEP_COUNT)
            {
                transitionAnimationStateComponent.mAscendingPalette = true;
            }
        }
    }
}

void TransitionAnimationSystem::UpdateEncounterTransitionAnimation(const float dt) const
{
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    //auto& encounterStateComponent           = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    transitionAnimationStateComponent.mAnimationTimer->Update(dt);
    if (transitionAnimationStateComponent.mAnimationTimer->HasTicked())
    {
        transitionAnimationStateComponent.mAnimationTimer->Reset();
        if (transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity != ecs::NULL_ENTITY_ID)
        {
            mWorld.RemoveEntity(transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity);
        }
        
        transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity = mWorld.CreateEntity();
        
        auto renderableComponent = std::make_unique<RenderableComponent>();
        renderableComponent->mTextureResourceId = transitionAnimationStateComponent.mAnimFrameResourceIdQueue.front();
        renderableComponent->mActiveAnimationNameId = StringId("default");
        renderableComponent->mShaderNameId          = StringId("gui");
        renderableComponent->mAffectedByPerspective = false;
        renderableComponent->mActiveAnimationNameId = StringId("default");
        
        const auto frameModelPath = ResourceLoadingService::RES_MODELS_ROOT + GUI_COMPONENTS_MODEL_NAME + ".obj";
        auto& resourceLoadingService = ResourceLoadingService::GetInstance();
        renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(frameModelPath));
        
        transitionAnimationStateComponent.mAnimFrameResourceIdQueue.pop();
        
        auto transformComponent = std::make_unique<TransformComponent>();
        transformComponent->mScale = glm::vec3(2.0f, 2.0f, 1.0f);
        
        mWorld.AddComponent<RenderableComponent>(transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity, std::move(renderableComponent));
        mWorld.AddComponent<TransformComponent>(transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity, std::move(transformComponent));
    }
}

void TransitionAnimationSystem::LoadEncounterSpecificAnimation() const
{
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    
    const auto transitionAnimationDirPath = ResourceLoadingService::RES_TEXTURES_ROOT + "transition_animations/wild_anim1/";
    const auto& encounterAnimFilenames = GetAllFilenamesInDirectory(transitionAnimationDirPath);
    for (const auto& filename: encounterAnimFilenames)
    {
        if (resourceLoadingService.HasLoadedResource(transitionAnimationDirPath + filename) == false)
        {
            transitionAnimationStateComponent.mAnimFrameResourceIdQueue.push(resourceLoadingService.LoadResource(transitionAnimationDirPath + filename));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
