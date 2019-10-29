//
//  TrainerCardOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TrainerCardOverworldFlowState.h"
#include "MainMenuOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TextboxComponent.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/PokemonSelectionViewSpriteUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../components/TransformComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string TrainerCardOverworldFlowState::TRAINER_BACKGROUND_SPRITE_MODEL_FILE_NAME = "trainer_card_sprite.obj";
const std::string TrainerCardOverworldFlowState::TRAINER_BACKGROUND_TEXTURE_FILE_NAME      = "trainer_card.png";

const glm::vec3 TrainerCardOverworldFlowState::BACKGROUND_POSITION = glm::vec3(0.0f, 0.0f, 0.01f);
const glm::vec3 TrainerCardOverworldFlowState::BACKGROUND_SCALE    = glm::vec3(2.0f, 2.0f, 2.0f);

const glm::vec3 TrainerCardOverworldFlowState::TRAINER_CARD_BACKGROUND_POSITION = glm::vec3(0.0f, -0.01f, -0.1f);
const glm::vec3 TrainerCardOverworldFlowState::TRAINER_CARD_BACKGROUND_SCALE    = glm::vec3(2.2f, 2.2f, 1.0f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TrainerCardOverworldFlowState::TrainerCardOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)   
{   
    CreateBackground();    
}

void TrainerCardOverworldFlowState::VUpdate(const float)
{    
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    
    if 
    (
        IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||
        IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent)
    )
    {
        DestroyBackground();
        CompleteAndTransitionTo<MainMenuOverworldFlowState>();
    }     
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void TrainerCardOverworldFlowState::CreateBackground()
{
    mBackgroundCoverEntityId = LoadAndCreatePokemonSelectionViewBackgroundCover
    (
        BACKGROUND_POSITION,
        BACKGROUND_SCALE,
        mWorld
    );

    mTrainerCardBackgroundEntityId = mWorld.CreateEntity();

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    auto renderableComponent     = std::make_unique<RenderableComponent>();

    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + TRAINER_BACKGROUND_TEXTURE_FILE_NAME;
    renderableComponent->mTextureResourceId = resourceLoadingService.LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;

    const auto modelPath = ResourceLoadingService::RES_MODELS_ROOT + TRAINER_BACKGROUND_SPRITE_MODEL_FILE_NAME;
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = TRAINER_CARD_BACKGROUND_POSITION;
    transformComponent->mScale    = TRAINER_CARD_BACKGROUND_SCALE;

    mWorld.AddComponent<RenderableComponent>(mTrainerCardBackgroundEntityId, std::move(renderableComponent));
    mWorld.AddComponent<TransformComponent>(mTrainerCardBackgroundEntityId, std::move(transformComponent));    
}

void TrainerCardOverworldFlowState::DestroyBackground()
{
    mWorld.DestroyEntity(mBackgroundCoverEntityId);
    mWorld.DestroyEntity(mTrainerCardBackgroundEntityId);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
