//
//  PewterMuseumKabutopsFossilOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 13/12/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PewterMuseumKabutopsFossilOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../rendering/components/RenderableComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PewterMuseumKabutopsFossilOverworldFlowState::KABUTOPS_IMAGE_TEXTURE_FILE_NAME = "kabutops_fossil.png";
const std::string PewterMuseumKabutopsFossilOverworldFlowState::KABUTOPS_IMAGE_MODEL_FILE_NAME   = "kabutops_fossil.obj";

const glm::vec3 PewterMuseumKabutopsFossilOverworldFlowState::KABUTOPS_FOSSIL_TEXTBOX_POSITION = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 PewterMuseumKabutopsFossilOverworldFlowState::KABUTOPS_IMAGE_POSITION          = glm::vec3(0.0f, 0.0f, -0.1f);
const glm::vec3 PewterMuseumKabutopsFossilOverworldFlowState::KABUTOPS_IMAGE_SCALE             = glm::vec3(0.49f, 0.49f, 1.0f);

const int PewterMuseumKabutopsFossilOverworldFlowState::KABUTOPS_FOSSIL_TEXTBOX_COLS = 9;
const int PewterMuseumKabutopsFossilOverworldFlowState::KABUTOPS_FOSSIL_TEXTBOX_ROWS = 10;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PewterMuseumKabutopsFossilOverworldFlowState::PewterMuseumKabutopsFossilOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mKabutopsFossilTextboxEntityId(ecs::NULL_ENTITY_ID)
    , mKabutopsImageEntityId(ecs::NULL_ENTITY_ID)
{
    DestroyActiveTextbox(mWorld);
    CreateKabutopsTextbox();
    LoadAndCreateKabutopsImage();
}

void PewterMuseumKabutopsFossilOverworldFlowState::VUpdate(const float)
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    if 
    (
        IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||
        IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        mWorld.DestroyEntity(mKabutopsImageEntityId);
        DestroyGenericOrBareTextbox(mKabutopsFossilTextboxEntityId, mWorld);
        QueueDialogForChatbox(CreateChatbox(mWorld), "KABUTOPS Fossil#A primitive and#rare POK^MON.", mWorld);
        CompleteOverworldFlow();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PewterMuseumKabutopsFossilOverworldFlowState::CreateKabutopsTextbox()
{
    mKabutopsFossilTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::GENERIC_TEXTBOX, 
        KABUTOPS_FOSSIL_TEXTBOX_COLS, 
        KABUTOPS_FOSSIL_TEXTBOX_ROWS, 
        KABUTOPS_FOSSIL_TEXTBOX_POSITION.x,
        KABUTOPS_FOSSIL_TEXTBOX_POSITION.y, 
        KABUTOPS_FOSSIL_TEXTBOX_POSITION.z,
        mWorld
    );
}

void PewterMuseumKabutopsFossilOverworldFlowState::LoadAndCreateKabutopsImage()
{
    mKabutopsImageEntityId = mWorld.CreateEntity();

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    auto renderableComponent = std::make_unique<RenderableComponent>();

    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + KABUTOPS_IMAGE_TEXTURE_FILE_NAME;
    renderableComponent->mTextureResourceId     = resourceLoadingService.LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;

    const auto modelPath = ResourceLoadingService::RES_MODELS_ROOT + KABUTOPS_IMAGE_MODEL_FILE_NAME;
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = KABUTOPS_IMAGE_POSITION;
    transformComponent->mScale    = KABUTOPS_IMAGE_SCALE;

    mWorld.AddComponent<RenderableComponent>(mKabutopsImageEntityId, std::move(renderableComponent));
    mWorld.AddComponent<TransformComponent>(mKabutopsImageEntityId, std::move(transformComponent));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
