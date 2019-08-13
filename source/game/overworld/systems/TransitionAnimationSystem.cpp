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
#include "../components/TransitionAnimationStateSingletonComponent.h"
#include "../components/WarpConnectionsSingletonComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/FileUtils.h"
#include "../../common/utils/Logging.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../resources/TextureResource.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string TransitionAnimationSystem::TRANSITION_ANIM_MODEL_FILE_NAME = "transition_anim_quad.obj";
const float TransitionAnimationSystem::WARP_TRANSITION_STEP_DURATION          = 0.13f;
const float TransitionAnimationSystem::WILD_FLASH_ANIMATION_STEP_DURATION     = 0.035f;
const float TransitionAnimationSystem::ENCOUNTER_ANIMATION_FRAME_DURATION     = 0.04f;
const float TransitionAnimationSystem::ENCOUNTER_ANIMATION_END_DELAY_DURATION = 1.0f;
const int TransitionAnimationSystem::TRANSITION_STEP_COUNT                    = 3;
const int TransitionAnimationSystem::WILD_FLASH_CYCLE_REPEAT_COUNT            = 3;

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
    auto& encounterStateComponent           = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();

    if (transitionAnimationStateComponent.mIsPlayingTransitionAnimation)
    {
        switch (transitionAnimationStateComponent.mTransitionAnimationType)
        {
            case TransitionAnimationType::WARP: UpdateWarpTransitionAnimation(dt); break;
            case TransitionAnimationType::WILD_FLASH: UpdateWildFlashTransitionAnimation(dt); break;
            case TransitionAnimationType::ENCOUNTER: UpdateEncounterTransitionAnimation(dt); break;
            case TransitionAnimationType::ENCOUNTER_INTRO_END_DELAY: UpdateEncounterIntroEndDelayAnimation(dt); break;
            case TransitionAnimationType::ENCOUNTER_END: UpdateEncounterEndTransitionAnimation(dt); break;
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
    else if (encounterStateComponent.mActiveEncounterType == EncounterType::TRAINER)
    {
        encounterStateComponent.mOverworldEncounterAnimationState       = OverworldEncounterAnimationState::ENCOUNTER_INTRO_ANIMATION;
        transitionAnimationStateComponent.mTransitionAnimationType      = TransitionAnimationType::ENCOUNTER;
        transitionAnimationStateComponent.mIsPlayingTransitionAnimation = true;
        transitionAnimationStateComponent.mAnimationTimer = std::make_unique<Timer>(ENCOUNTER_ANIMATION_FRAME_DURATION);
        LoadEncounterSpecificAnimation();
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
            if (++transitionAnimationStateComponent.mAnimationProgressionStep >= TRANSITION_STEP_COUNT)
            {
                transitionAnimationStateComponent.mAscendingPalette = false;
            }
            
            if (transitionAnimationStateComponent.mAnimationProgressionStep == 0)
            {
                if (++transitionAnimationStateComponent.mAnimationCycleCompletionCount == WILD_FLASH_CYCLE_REPEAT_COUNT)
                {
                    encounterStateComponent.mOverworldEncounterAnimationState = OverworldEncounterAnimationState::ENCOUNTER_INTRO_ANIMATION;

                    transitionAnimationStateComponent.mTransitionAnimationType = TransitionAnimationType::ENCOUNTER;
                    transitionAnimationStateComponent.mAnimationTimer          = std::make_unique<Timer>(ENCOUNTER_ANIMATION_FRAME_DURATION);
                    LoadEncounterSpecificAnimation();
                }
            }
        }
        else
        {
            if (--transitionAnimationStateComponent.mAnimationProgressionStep <= -TRANSITION_STEP_COUNT)
            {
                transitionAnimationStateComponent.mAscendingPalette = true;
            }
        }                
    }
}

void TransitionAnimationSystem::UpdateEncounterTransitionAnimation(const float dt) const
{
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();

    transitionAnimationStateComponent.mAnimationTimer->Update(dt);
    if (transitionAnimationStateComponent.mAnimationTimer->HasTicked())
    {
        transitionAnimationStateComponent.mAnimationTimer->Reset();
        if (transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity != ecs::NULL_ENTITY_ID && transitionAnimationStateComponent.mAnimFrameResourceIdQueue.size() > 0)
        {            
            mWorld.DestroyEntity(transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity);
            transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity = ecs::NULL_ENTITY_ID;
        }
        
        if (transitionAnimationStateComponent.mAnimFrameResourceIdQueue.size() > 0)
        {
            transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity = mWorld.CreateEntity();

            auto renderableComponent = std::make_unique<RenderableComponent>();
            renderableComponent->mTextureResourceId     = transitionAnimationStateComponent.mAnimFrameResourceIdQueue.front();
            renderableComponent->mActiveAnimationNameId = StringId("default");
            renderableComponent->mShaderNameId          = StringId("gui");
            renderableComponent->mAffectedByPerspective = false;

            const auto frameModelPath = ResourceLoadingService::RES_MODELS_ROOT + TRANSITION_ANIM_MODEL_FILE_NAME;
            auto& resourceLoadingService = ResourceLoadingService::GetInstance();
            renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(frameModelPath));

            transitionAnimationStateComponent.mAnimFrameResourceIdQueue.pop();

            auto transformComponent = std::make_unique<TransformComponent>();
            transformComponent->mScale = glm::vec3(2.0f, 2.0f, 2.0f);

            mWorld.AddComponent<RenderableComponent>(transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity, std::move(renderableComponent));
            mWorld.AddComponent<TransformComponent>(transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity, std::move(transformComponent));
        }
        else
        {
            transitionAnimationStateComponent.mTransitionAnimationType = TransitionAnimationType::ENCOUNTER_INTRO_END_DELAY;
            transitionAnimationStateComponent.mAnimationTimer          = std::make_unique<Timer>(ENCOUNTER_ANIMATION_END_DELAY_DURATION);
        }           
    }
}

void TransitionAnimationSystem::UpdateEncounterIntroEndDelayAnimation(const float dt) const
{
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    auto& encounterStateComponent           = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    transitionAnimationStateComponent.mAnimationTimer->Update(dt);
    if (transitionAnimationStateComponent.mAnimationTimer->HasTicked())
    {        
        transitionAnimationStateComponent.mAnimationTimer->Reset();

        if (transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity != ecs::NULL_ENTITY_ID)
        {
            mWorld.DestroyEntity(transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity);
            transitionAnimationStateComponent.mEncounterSpecificAnimFrameEntity = ecs::NULL_ENTITY_ID;
        }
                     
        transitionAnimationStateComponent.mAnimationProgressionStep     = 0;
        encounterStateComponent.mOverworldEncounterAnimationState       = OverworldEncounterAnimationState::ENCOUNTER_INTRO_ANIMATION_COMPLETE;
    }
}

void TransitionAnimationSystem::UpdateEncounterEndTransitionAnimation(const float dt) const
{
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    
    transitionAnimationStateComponent.mAnimationTimer->Update(dt);
    if (transitionAnimationStateComponent.mAnimationTimer->HasTicked())
    {
        transitionAnimationStateComponent.mAnimationTimer->Reset();
        
        if (transitionAnimationStateComponent.mAnimationProgressionStep == 0)
        {
            transitionAnimationStateComponent.mTransitionAnimationType      = TransitionAnimationType::WARP;
            transitionAnimationStateComponent.mIsPlayingTransitionAnimation = false;
            transitionAnimationStateComponent.mAnimationProgressionStep     = 0;
        }
        else if (transitionAnimationStateComponent.mAnimationProgressionStep < 0)
        {
            transitionAnimationStateComponent.mAnimationProgressionStep++;
        }
        else
        {
            transitionAnimationStateComponent.mAnimationProgressionStep--;
        }
    }
}

void TransitionAnimationSystem::LoadEncounterSpecificAnimation() const
{
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
        
    auto transitionAnimationDirPath = SelectAppropriateBattleTransitionAnimation();
    const auto& encounterAnimFilenames = GetAllFilenamesInDirectory(transitionAnimationDirPath);
    for (const auto& filename: encounterAnimFilenames)
    {        
        transitionAnimationStateComponent.mAnimFrameResourceIdQueue.push(resourceLoadingService.LoadResource(transitionAnimationDirPath + filename));        
    }
}

std::string TransitionAnimationSystem::SelectAppropriateBattleTransitionAnimation() const
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
    {
        return ResourceLoadingService::RES_TEXTURES_ROOT + "transition_animations/wild_anim1/";
    }
    else
    {
        return ResourceLoadingService::RES_TEXTURES_ROOT + "transition_animations/trainer_anim1/";
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
