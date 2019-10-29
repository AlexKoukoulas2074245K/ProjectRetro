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
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/MeshUtils.h"
#include "../components/TransformComponent.h"

#include <array>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string TrainerCardOverworldFlowState::TRAINER_BACKGROUND_SPRITE_MODEL_FILE_NAME = "trainer_card_sprite.obj";
const std::string TrainerCardOverworldFlowState::TRAINER_BACKGROUND_TEXTURE_FILE_NAME      = "trainer_card.png";
const std::string TrainerCardOverworldFlowState::ENCOUNTER_SPRITE_MODEL_NAME               = "camera_facing_quad_hud_sub_atlas";
const std::string TrainerCardOverworldFlowState::ENCOUNTER_SPRITE_ANIMATION_NAME           = "default";
const std::string TrainerCardOverworldFlowState::ENCOUNTER_SPRITE_SHADER_NAME              = "gui";
const std::string TrainerCardOverworldFlowState::EMBLEMS_ATLAS_FILE_NAME                   = "emblems.png";

const glm::vec3 TrainerCardOverworldFlowState::BACKGROUND_POSITION = glm::vec3(0.0f, 0.0f, 0.01f);
const glm::vec3 TrainerCardOverworldFlowState::BACKGROUND_SCALE    = glm::vec3(2.0f, 2.0f, 2.0f);

const glm::vec3 TrainerCardOverworldFlowState::TRAINER_CARD_BACKGROUND_POSITION = glm::vec3(0.0f, 0.0f, -0.1f);
const glm::vec3 TrainerCardOverworldFlowState::TRAINER_CARD_BACKGROUND_SCALE    = glm::vec3(2.2f, 2.2f, 1.0f);
const glm::vec3 TrainerCardOverworldFlowState::INFO_TEXTBOX_POSITION            = glm::vec3(0.0684000328f, 0.495599777f, -0.2f);
const glm::vec3 TrainerCardOverworldFlowState::EMBLEM_SPRITE_ORIGIN_POSITION    = glm::vec3(-0.425399810f, -0.390099853f, -0.2f);
const glm::vec3 TrainerCardOverworldFlowState::EMBLEM_SPRITE_SCALE              = glm::vec3(0.266924173f, 0.266924173f, 1.0f);

const int TrainerCardOverworldFlowState::INFO_TEXTBOX_COLS = 8;
const int TrainerCardOverworldFlowState::INFO_TEXTBOX_ROWS = 5;
const int TrainerCardOverworldFlowState::EMBLEM_ATALS_COLS = 4;
const int TrainerCardOverworldFlowState::EMBLEM_ATALS_ROWS = 2;

const float TrainerCardOverworldFlowState::EMBLEMS_HOR_DISTANCE = 0.278799921f;
const float TrainerCardOverworldFlowState::EMBLEMS_VER_DISTANCE = 0.321399391f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TrainerCardOverworldFlowState::TrainerCardOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)   
{       
    CreateBackground();       
    CreateInfoTextbox();   
    CreateEmblems();
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
        DestroyEmblems();
        DestroyInfoTextbox();
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

void TrainerCardOverworldFlowState::CreateInfoTextbox()
{
    mInfoTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::BARE_TEXTBOX, 
        INFO_TEXTBOX_COLS, 
        INFO_TEXTBOX_ROWS, 
        INFO_TEXTBOX_POSITION.x,
        INFO_TEXTBOX_POSITION.y,
        INFO_TEXTBOX_POSITION.z,
        mWorld
    );

    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    WriteTextAtTextboxCoords(mInfoTextboxEntityId, playerStateComponent.mPlayerTrainerName.GetString(), 0, 0, mWorld);
    WriteTextAtTextboxCoords(mInfoTextboxEntityId, "$" + std::to_string(playerStateComponent.mPokeDollarCredits), 1, 2, mWorld);
    WriteTextAtTextboxCoords(mInfoTextboxEntityId, GetHoursMinutesStringFromSeconds(playerStateComponent.mSecondsPlayed), 2, 4, mWorld);

}

void TrainerCardOverworldFlowState::CreateEmblems()
{
    static const std::array<std::array<unsigned long, EMBLEM_ATALS_COLS>, EMBLEM_ATALS_ROWS> sEmblemMasks = 
    { 
        { 
            { { milestones::BOULDERBADGE, milestones::CASCADEBADGE, milestones::THUNDERBADGE, milestones::RAINBOWBADGE }  },
            { { milestones::SOULBADGE,    milestones::MARSHBADGE,   milestones::VOLCANOBADGE, milestones::EARTHBADGE   }  }
        } 
    };

    for (size_t row = 0U; row < sEmblemMasks.size(); ++row)
    {
        const auto& emblemMaskRow = sEmblemMasks[row];
        for (size_t col = 0U; col < emblemMaskRow.size(); ++col)
        {
            if (HasMilestone(emblemMaskRow[col], mWorld))
            {
                const auto emblemSpriteEntityId = mWorld.CreateEntity();

                auto renderableComponent = std::make_unique<RenderableComponent>();
                renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + EMBLEMS_ATLAS_FILE_NAME);
                renderableComponent->mActiveAnimationNameId = StringId(ENCOUNTER_SPRITE_ANIMATION_NAME);
                renderableComponent->mShaderNameId          = StringId(ENCOUNTER_SPRITE_SHADER_NAME);
                renderableComponent->mAffectedByPerspective = false;

                LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations
                (
                    col,
                    row,
                    EMBLEM_ATALS_COLS,
                    EMBLEM_ATALS_ROWS,
                    false,
                    ENCOUNTER_SPRITE_MODEL_NAME,
                    renderableComponent->mActiveAnimationNameId,
                    *renderableComponent
                );

                auto transformComponent          = std::make_unique<TransformComponent>();
                transformComponent->mPosition    = EMBLEM_SPRITE_ORIGIN_POSITION;
                transformComponent->mPosition.x += EMBLEMS_HOR_DISTANCE * col;
                transformComponent->mPosition.y -= EMBLEMS_VER_DISTANCE * row;
                transformComponent->mScale       = EMBLEM_SPRITE_SCALE;

                mWorld.AddComponent<RenderableComponent>(emblemSpriteEntityId, std::move(renderableComponent));
                mWorld.AddComponent<TransformComponent>(emblemSpriteEntityId, std::move(transformComponent));

                mEmblemEntityIds.push_back(emblemSpriteEntityId);
            }
        }
    }    
}

void TrainerCardOverworldFlowState::DestroyBackground()
{
    mWorld.DestroyEntity(mBackgroundCoverEntityId);
    mWorld.DestroyEntity(mTrainerCardBackgroundEntityId);
}

void TrainerCardOverworldFlowState::DestroyInfoTextbox()
{
    DestroyGenericOrBareTextbox(mInfoTextboxEntityId, mWorld);
}

void TrainerCardOverworldFlowState::DestroyEmblems()
{
    for (const auto entityId : mEmblemEntityIds)
    {
        mWorld.DestroyEntity(entityId);
    }

    mEmblemEntityIds.clear();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
