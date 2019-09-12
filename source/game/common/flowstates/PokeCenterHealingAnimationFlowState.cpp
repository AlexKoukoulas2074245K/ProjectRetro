//
//  PokeCenterHealingAnimationFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokeCenterHealingAnimationFlowState.h"
#include "../components/TransformComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/LevelResidentComponent.h"
#include "../../overworld/components/PokeCenterHealingAnimationStateSingletonComponent.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PokeCenterHealingAnimationFlowState::PC_OVERLAY_POSITION = glm::vec3(16.9f, 0.0f, 17.455f);

const std::string PokeCenterHealingAnimationFlowState::PC_OVERLAY_MODEL_NAME = "in_poke_center_computer_screen_overlay";

const int PokeCenterHealingAnimationFlowState::JOY_NPC_LEVEL_INDEX = 0;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokeCenterHealingAnimationFlowState::PokeCenterHealingAnimationFlowState(ecs::World& world)
    : BaseFlowState(world)
{   
    auto& pokeCenterHealingAnimationState = mWorld.GetSingletonComponent<PokeCenterHealingAnimationStateSingletonComponent>();
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::ANIMATION_START);
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::JOY_FACING_NORTH);
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::JOY_FACING_WEST);
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::POKEBALL_PLACEMENT);
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::COLOR_SWAPPING);
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::HEALING_FINISHED_JOY_FACING_NORTH);
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::THANK_YOU_DIALOG);
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::JOY_BOW);
    
    SetCurrentStateDurationTimer();    
}

void PokeCenterHealingAnimationFlowState::VUpdate(const float dt)
{
    auto& pokeCenterHealingAnimationState = mWorld.GetSingletonComponent<PokeCenterHealingAnimationStateSingletonComponent>();
    switch (pokeCenterHealingAnimationState.mHealingAnimationStateQueue.front())
    {
        case PokeCenterHealingAnimationState::ANIMATION_START: 
        {
            pokeCenterHealingAnimationState.mAnimationTimer->Update(dt);
            if (pokeCenterHealingAnimationState.mAnimationTimer->HasTicked())
            {
                pokeCenterHealingAnimationState.mHealingAnimationStateQueue.pop();
                SetCurrentStateDurationTimer();
            }
        } break;
        case PokeCenterHealingAnimationState::JOY_FACING_NORTH: 
        {
            const auto joyEntityId = GetJoyEntityId();
            auto& joyRenderableComponent = mWorld.GetComponent<RenderableComponent>(joyEntityId);
            ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::NORTH), joyRenderableComponent);

            pokeCenterHealingAnimationState.mAnimationTimer->Update(dt);
            if (pokeCenterHealingAnimationState.mAnimationTimer->HasTicked())
            {
                pokeCenterHealingAnimationState.mHealingAnimationStateQueue.pop();
                SetCurrentStateDurationTimer();
            }
        } break;

        case PokeCenterHealingAnimationState::JOY_FACING_WEST:
        {
            const auto joyEntityId = GetJoyEntityId();
            auto& joyRenderableComponent = mWorld.GetComponent<RenderableComponent>(joyEntityId);
            ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::WEST), joyRenderableComponent);

            pokeCenterHealingAnimationState.mAnimationTimer->Update(dt);
            if (pokeCenterHealingAnimationState.mAnimationTimer->HasTicked())
            {
                SoundService::GetInstance().MuteMusic();
                ShowComputerScreenOverlayEffect(ComputerScreenOverlayEffect::WHITE);
                pokeCenterHealingAnimationState.mHealingAnimationStateQueue.pop();
                SetCurrentStateDurationTimer();
            }
        } break;
        
        case PokeCenterHealingAnimationState::POKEBALL_PLACEMENT:
        {
            
            
        } break;
        case PokeCenterHealingAnimationState::COLOR_SWAPPING:
        case PokeCenterHealingAnimationState::HEALING_FINISHED_JOY_FACING_NORTH:
        case PokeCenterHealingAnimationState::THANK_YOU_DIALOG:
        case PokeCenterHealingAnimationState::JOY_BOW: break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId PokeCenterHealingAnimationFlowState::GetJoyEntityId() const
{
    return GetNpcEntityIdFromLevelIndex(JOY_NPC_LEVEL_INDEX, mWorld);
}

void PokeCenterHealingAnimationFlowState::ShowComputerScreenOverlayEffect(const ComputerScreenOverlayEffect computerScreenOverlayEffect) const
{
    auto& pokeCenterHealingAnimationState = mWorld.GetSingletonComponent<PokeCenterHealingAnimationStateSingletonComponent>();
    if (pokeCenterHealingAnimationState.mComputerScreenOverlayEntityId != ecs::NULL_ENTITY_ID)
    {
        mWorld.DestroyEntity(pokeCenterHealingAnimationState.mComputerScreenOverlayEntityId);
        pokeCenterHealingAnimationState.mComputerScreenOverlayEntityId = ecs::NULL_ENTITY_ID;
    }

    if (computerScreenOverlayEffect == ComputerScreenOverlayEffect::NONE) return;

    pokeCenterHealingAnimationState.mComputerScreenOverlayEntityId = mWorld.CreateEntity();

    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = PC_OVERLAY_POSITION;
    
    auto renderableComponent = std::make_unique<RenderableComponent>();
    renderableComponent->mRenderableLayer = RenderableLayer::LEVEL_FLOOR_LEVEL;
    renderableComponent->mShaderNameId = StringId("basic");
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back
    (
        ResourceLoadingService::GetInstance().
        LoadResource(ResourceLoadingService::RES_MODELS_ROOT + PC_OVERLAY_MODEL_NAME + ".obj")
    );
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource
    (
        ResourceLoadingService::RES_TEXTURES_ROOT + PC_OVERLAY_MODEL_NAME + (computerScreenOverlayEffect == ComputerScreenOverlayEffect::BLUE ? "_blue" : "_white") + ".png"
    );

    auto levelResidentComponent = std::make_unique<LevelResidentComponent>();
    levelResidentComponent->mLevelNameId = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>().mActiveLevelNameId;

    mWorld.AddComponent<TransformComponent>(pokeCenterHealingAnimationState.mComputerScreenOverlayEntityId, std::move(transformComponent));
    mWorld.AddComponent<LevelResidentComponent>(pokeCenterHealingAnimationState.mComputerScreenOverlayEntityId, std::move(levelResidentComponent));
    mWorld.AddComponent<RenderableComponent>(pokeCenterHealingAnimationState.mComputerScreenOverlayEntityId, std::move(renderableComponent));
}

void PokeCenterHealingAnimationFlowState::SetCurrentStateDurationTimer() const
{
    auto& pokeCenterHealingAnimationState = mWorld.GetSingletonComponent<PokeCenterHealingAnimationStateSingletonComponent>();
    pokeCenterHealingAnimationState.mAnimationTimer = std::make_unique<Timer>(sHealingAnimationStateDurations.at(pokeCenterHealingAnimationState.mHealingAnimationStateQueue.front()));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

