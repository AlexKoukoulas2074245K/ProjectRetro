//
//  PewterMuseumAerodactylFossilOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/12/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PewterMuseumAerodactylFossilOverworldFlowState.h"
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

const std::string PewterMuseumAerodactylFossilOverworldFlowState::AERODACTYL_IMAGE_TEXTURE_FILE_NAME = "aerodactyl_fossil.png";
const std::string PewterMuseumAerodactylFossilOverworldFlowState::AERODACTYL_IMAGE_MODEL_FILE_NAME = "aerodactyl_fossil.obj";

const glm::vec3 PewterMuseumAerodactylFossilOverworldFlowState::AERODACTYL_FOSSIL_TEXTBOX_POSITION = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 PewterMuseumAerodactylFossilOverworldFlowState::AERODACTYL_IMAGE_POSITION = glm::vec3(0.0f, 0.0f, -0.1f);
const glm::vec3 PewterMuseumAerodactylFossilOverworldFlowState::AERODACTYL_IMAGE_SCALE = glm::vec3(0.49f, 0.49f, 1.0f);

const int PewterMuseumAerodactylFossilOverworldFlowState::AERODACTYL_FOSSIL_TEXTBOX_COLS = 9;
const int PewterMuseumAerodactylFossilOverworldFlowState::AERODACTYL_FOSSIL_TEXTBOX_ROWS = 10;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PewterMuseumAerodactylFossilOverworldFlowState::PewterMuseumAerodactylFossilOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mAerodactylFossilTextboxEntityId(ecs::NULL_ENTITY_ID)
    , mAerodactylImageEntityId(ecs::NULL_ENTITY_ID)
{
    DestroyActiveTextbox(mWorld);
    CreateAerodactylTextbox();
    LoadAndCreateAerodactylImage();
}

void PewterMuseumAerodactylFossilOverworldFlowState::VUpdate(const float)
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    if 
    (
        IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||
        IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        mWorld.DestroyEntity(mAerodactylImageEntityId);
        DestroyGenericOrBareTextbox(mAerodactylFossilTextboxEntityId, mWorld);
        QueueDialogForChatbox(CreateChatbox(mWorld), "AERODACTYL Fossil#A primitive and#rare POK^MON.", mWorld);
        CompleteOverworldFlow();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PewterMuseumAerodactylFossilOverworldFlowState::CreateAerodactylTextbox()
{
    mAerodactylFossilTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::GENERIC_TEXTBOX, 
        AERODACTYL_FOSSIL_TEXTBOX_COLS, 
        AERODACTYL_FOSSIL_TEXTBOX_ROWS, 
        AERODACTYL_FOSSIL_TEXTBOX_POSITION.x,
        AERODACTYL_FOSSIL_TEXTBOX_POSITION.y, 
        AERODACTYL_FOSSIL_TEXTBOX_POSITION.z,
        mWorld
    );
}

void PewterMuseumAerodactylFossilOverworldFlowState::LoadAndCreateAerodactylImage()
{
    mAerodactylImageEntityId = mWorld.CreateEntity();

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    auto renderableComponent = std::make_unique<RenderableComponent>();

    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + AERODACTYL_IMAGE_TEXTURE_FILE_NAME;
    renderableComponent->mTextureResourceId     = resourceLoadingService.LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;

    const auto modelPath = ResourceLoadingService::RES_MODELS_ROOT + AERODACTYL_IMAGE_MODEL_FILE_NAME;
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = AERODACTYL_IMAGE_POSITION;
    transformComponent->mScale    = AERODACTYL_IMAGE_SCALE;

    mWorld.AddComponent<RenderableComponent>(mAerodactylImageEntityId, std::move(renderableComponent));
    mWorld.AddComponent<TransformComponent>(mAerodactylImageEntityId, std::move(transformComponent));    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
