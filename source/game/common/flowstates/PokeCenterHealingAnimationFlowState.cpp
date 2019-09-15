//
//  PokeCenterHealingAnimationFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokeCenterHealingFarewellDialogFlowState.h"
#include "PokeCenterHealingAnimationFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/LevelResidentComponent.h"
#include "../../overworld/components/PokeCenterHealingAnimationStateSingletonComponent.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../resources/MeshUtils.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PokeCenterHealingAnimationFlowState::PC_OVERLAY_POSITION         = glm::vec3(16.9049511f, 0.00630000001f, 17.4549999f);
const glm::vec3 PokeCenterHealingAnimationFlowState::FIRST_HEALING_BALL_POSITION = glm::vec3(17.0f, 0.523000658f, 15.0216150f);

const StringId PokeCenterHealingAnimationFlowState::JOY_BOW_ANIMATION_NAME = StringId("bow");

const std::string PokeCenterHealingAnimationFlowState::HEALING_MACHINE_MODEL_NAME   = "in_poke_center_healing_machine";
const std::string PokeCenterHealingAnimationFlowState::HEALING_BALL_MODEL_NAME      = "healing_ball";
const std::string PokeCenterHealingAnimationFlowState::POKEBALL_HEALING_SFX_NAME    = "general/pokeball_healing";
const std::string PokeCenterHealingAnimationFlowState::HEALING_ANIMATION_SFX_NAME   = "general/healing_long";
const std::string PokeCenterHealingAnimationFlowState::POKE_CENTER_MUSIC_TRACK_NAME = "poke_center_mart";
const std::string PokeCenterHealingAnimationFlowState::CHARACTER_MODEL_NAME         = "camera_facing_quad";

const float PokeCenterHealingAnimationFlowState::HEALING_BALL_X_DISTANCE = 0.5799007f;
const float PokeCenterHealingAnimationFlowState::HEALING_BALL_Z_DISTANCE = 0.4590192f;

const int PokeCenterHealingAnimationFlowState::JOY_NPC_LEVEL_INDEX            = 0;
const int PokeCenterHealingAnimationFlowState::JOY_BOW_SPRITE_ATLAS_COL       = 7;
const int PokeCenterHealingAnimationFlowState::JOY_BOW_SPRITE_ATLAS_ROW       = 4;
const int PokeCenterHealingAnimationFlowState::FIRST_HEALING_MACHINE_GAME_COL = 11;
const int PokeCenterHealingAnimationFlowState::FIRST_HEALING_MACHINE_GAME_ROW = 10;
const int PokeCenterHealingAnimationFlowState::CHARACTER_ATLAS_COLS           = 8;
const int PokeCenterHealingAnimationFlowState::CHARACTER_ATLAS_ROWS           = 64;

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
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::HEALING_FINISHED_DELAY);
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::HEALING_FINISHED_JOY_FACING_NORTH);
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::THANK_YOU_DIALOG);
    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.push(PokeCenterHealingAnimationState::JOY_BOW);

    pokeCenterHealingAnimationState.mPokeballEntityIds.clear();    

    pokeCenterHealingAnimationState.mFlashingCounter = 0;
    pokeCenterHealingAnimationState.mInvertedColors  = false;

    AddBowAnimationToJoy();
    PreloadHealingMachineSkins();
    SetCurrentStateDurationTimer();    
}

void PokeCenterHealingAnimationFlowState::VUpdate(const float dt)
{    
    const auto& playerStateComponent      = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
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
            pokeCenterHealingAnimationState.mAnimationTimer->Update(dt);
            if (pokeCenterHealingAnimationState.mAnimationTimer->HasTicked())
            {
                pokeCenterHealingAnimationState.mAnimationTimer->Reset();
                
                if (pokeCenterHealingAnimationState.mPokeballEntityIds.size() != playerStateComponent.mPlayerPokemonRoster.size())
                {
                    SoundService::GetInstance().PlaySfx(POKEBALL_HEALING_SFX_NAME);
                    ShowHealingBallWithIndex(pokeCenterHealingAnimationState.mPokeballEntityIds.size(), false);
                }
                else
                {                    
                    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.pop();
                    SetCurrentStateDurationTimer();
                }
            }
        } break;

        case PokeCenterHealingAnimationState::COLOR_SWAPPING:
        {
            pokeCenterHealingAnimationState.mAnimationTimer->Update(dt);
            if (pokeCenterHealingAnimationState.mAnimationTimer->HasTicked())
            {
                pokeCenterHealingAnimationState.mAnimationTimer->Reset();
                if (pokeCenterHealingAnimationState.mFlashingCounter == 0)
                {
                    SoundService::GetInstance().PlaySfx(HEALING_ANIMATION_SFX_NAME);
                }

                if (pokeCenterHealingAnimationState.mFlashingCounter++ != 8)
                {
                    pokeCenterHealingAnimationState.mInvertedColors = !pokeCenterHealingAnimationState.mInvertedColors;

                    for (auto i = 0U; i < playerStateComponent.mPlayerPokemonRoster.size(); ++i)
                    {
                        ShowHealingBallWithIndex(i, pokeCenterHealingAnimationState.mInvertedColors);
                    }

                    ShowComputerScreenOverlayEffect(pokeCenterHealingAnimationState.mInvertedColors ? ComputerScreenOverlayEffect::WHITE : ComputerScreenOverlayEffect::BLUE);
                }
                else
                {                                     
                    for (const auto entityId : pokeCenterHealingAnimationState.mPokeballEntityIds)
                    {
                        if (entityId != ecs::NULL_ENTITY_ID)
                        {
                            mWorld.DestroyEntity(entityId);
                        }
                    }

                    ShowComputerScreenOverlayEffect(ComputerScreenOverlayEffect::NONE);

                    pokeCenterHealingAnimationState.mHealingAnimationStateQueue.pop();
                    SetCurrentStateDurationTimer();
                }
            }
        } break;
        case PokeCenterHealingAnimationState::HEALING_FINISHED_DELAY:
        {
            pokeCenterHealingAnimationState.mAnimationTimer->Update(dt);
            if (pokeCenterHealingAnimationState.mAnimationTimer->HasTicked())
            {
                pokeCenterHealingAnimationState.mHealingAnimationStateQueue.pop();
                SetCurrentStateDurationTimer();
            }
        } break;
        case PokeCenterHealingAnimationState::HEALING_FINISHED_JOY_FACING_NORTH:
        {
            const auto joyEntityId = GetJoyEntityId();
            auto& joyRenderableComponent = mWorld.GetComponent<RenderableComponent>(joyEntityId);
            ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::NORTH), joyRenderableComponent);

            pokeCenterHealingAnimationState.mAnimationTimer->Update(dt);
            if (pokeCenterHealingAnimationState.mAnimationTimer->HasTicked())
            {
                ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::SOUTH), joyRenderableComponent);

                pokeCenterHealingAnimationState.mAnimationTimer->Reset();
                SoundService::GetInstance().PlayMusic(POKE_CENTER_MUSIC_TRACK_NAME, false);

                DestroyActiveTextbox(mWorld);
                QueueDialogForChatbox(CreateChatbox(mWorld), "Thank you!#Your POK^MON are#fighting fit!#+END", mWorld);

                pokeCenterHealingAnimationState.mHealingAnimationStateQueue.pop();
                SetCurrentStateDurationTimer();
            }
        } break;
        case PokeCenterHealingAnimationState::THANK_YOU_DIALOG: 
        {           
            if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
            {                
                auto& joyRenderableComponent = mWorld.GetComponent<RenderableComponent>(GetJoyEntityId());
                ChangeAnimationIfCurrentPlayingIsDifferent(JOY_BOW_ANIMATION_NAME, joyRenderableComponent);

                pokeCenterHealingAnimationState.mHealingAnimationStateQueue.pop();
                SetCurrentStateDurationTimer();
            }
        } break;

        case PokeCenterHealingAnimationState::JOY_BOW: 
        {            
            pokeCenterHealingAnimationState.mAnimationTimer->Update(dt);
            if (pokeCenterHealingAnimationState.mAnimationTimer->HasTicked())
            {                
                auto& joyRenderableComponent = mWorld.GetComponent<RenderableComponent>(GetJoyEntityId());
                ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::SOUTH), joyRenderableComponent);

                for (auto& pokemon : playerStateComponent.mPlayerPokemonRoster)
                {
                    RestorePokemonStats(*pokemon);
                }

                pokeCenterHealingAnimationState.mHealingAnimationStateQueue.pop();
                CompleteAndTransitionTo<PokeCenterHealingFarewellDialogFlowState>();
            }
        } break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId PokeCenterHealingAnimationFlowState::GetJoyEntityId() const
{
    return GetNpcEntityIdFromLevelIndex(JOY_NPC_LEVEL_INDEX, mWorld);
}

ecs::EntityId PokeCenterHealingAnimationFlowState::GetHealingMachineEntityId() const
{
    const auto& activeEntities = mWorld.GetActiveEntities();
    const auto healingMachinePosition = TileCoordsToPosition(FIRST_HEALING_MACHINE_GAME_COL, FIRST_HEALING_MACHINE_GAME_ROW);

    for (const auto& entityId : activeEntities)
    {
        if (mWorld.HasComponent<TransformComponent>(entityId))
        {
            const auto& transformComponent = mWorld.GetComponent<TransformComponent>(entityId);
            if 
            (
                math::Abs(healingMachinePosition.x - transformComponent.mPosition.x) < 0.01f &&
                math::Abs(healingMachinePosition.y - transformComponent.mPosition.y) < 0.01f &&
                math::Abs(healingMachinePosition.z - transformComponent.mPosition.z) < 0.01f
            )
            {
                return entityId;
            }
        }
    }

    return ecs::NULL_ENTITY_ID;
}

void PokeCenterHealingAnimationFlowState::ShowComputerScreenOverlayEffect(const ComputerScreenOverlayEffect computerScreenOverlayEffect) const
{            
    auto healingMachineTexturePath = ResourceLoadingService::RES_TEXTURES_ROOT + HEALING_MACHINE_MODEL_NAME;

    if (computerScreenOverlayEffect == ComputerScreenOverlayEffect::WHITE)
    {
        healingMachineTexturePath += "_white";
    }
    else if (computerScreenOverlayEffect == ComputerScreenOverlayEffect::BLUE)
    {
        healingMachineTexturePath += "_blue";
    }

    healingMachineTexturePath += ".png";

    auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(GetHealingMachineEntityId());
    renderableComponent.mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource(healingMachineTexturePath);            
}

void PokeCenterHealingAnimationFlowState::ShowHealingBallWithIndex(const size_t ballIndex, const bool invertedColors) const
{
    auto& pokeCenterHealingAnimationState = mWorld.GetSingletonComponent<PokeCenterHealingAnimationStateSingletonComponent>();
    if (pokeCenterHealingAnimationState.mPokeballEntityIds.size() > ballIndex)
    {
        mWorld.DestroyEntity(pokeCenterHealingAnimationState.mPokeballEntityIds[ballIndex]);
        pokeCenterHealingAnimationState.mPokeballEntityIds[ballIndex] = ecs::NULL_ENTITY_ID;
    }
    else
    {
        pokeCenterHealingAnimationState.mPokeballEntityIds.push_back(ecs::NULL_ENTITY_ID);
    }

    pokeCenterHealingAnimationState.mPokeballEntityIds[ballIndex] = mWorld.CreateEntity();

    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = FIRST_HEALING_BALL_POSITION;        
    transformComponent->mPosition.x += ballIndex % 2 == 1 ? HEALING_BALL_X_DISTANCE : 0.0f;        
    transformComponent->mPosition.z -= HEALING_BALL_Z_DISTANCE * (ballIndex / 2);

    auto renderableComponent = std::make_unique<RenderableComponent>();
    renderableComponent->mRenderableLayer = RenderableLayer::LEVEL_FLOOR_LEVEL;
    renderableComponent->mShaderNameId = StringId("basic");
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back
    (
        ResourceLoadingService::GetInstance().
        LoadResource(ResourceLoadingService::RES_MODELS_ROOT + HEALING_BALL_MODEL_NAME + ".obj")
    );
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource
    (
        ResourceLoadingService::RES_TEXTURES_ROOT + HEALING_BALL_MODEL_NAME + (invertedColors ? "_1" : "_2") + ".png"
    );

    auto levelResidentComponent = std::make_unique<LevelResidentComponent>();
    levelResidentComponent->mLevelNameId = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>().mActiveLevelNameId;

    mWorld.AddComponent<TransformComponent>(pokeCenterHealingAnimationState.mPokeballEntityIds[ballIndex], std::move(transformComponent));
    mWorld.AddComponent<LevelResidentComponent>(pokeCenterHealingAnimationState.mPokeballEntityIds[ballIndex], std::move(levelResidentComponent));
    mWorld.AddComponent<RenderableComponent>(pokeCenterHealingAnimationState.mPokeballEntityIds[ballIndex], std::move(renderableComponent));
}

void PokeCenterHealingAnimationFlowState::AddBowAnimationToJoy() const
{
    const auto joyEntityId = GetJoyEntityId();
    auto& joyRenderableComponent = mWorld.GetComponent<RenderableComponent>(joyEntityId);

    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations(JOY_BOW_SPRITE_ATLAS_COL, JOY_BOW_SPRITE_ATLAS_ROW, CHARACTER_ATLAS_COLS, CHARACTER_ATLAS_ROWS, false, CHARACTER_MODEL_NAME, JOY_BOW_ANIMATION_NAME, joyRenderableComponent);
}

void PokeCenterHealingAnimationFlowState::PreloadHealingMachineSkins() const
{    
    ResourceLoadingService::GetInstance().LoadResource
    (
        ResourceLoadingService::RES_TEXTURES_ROOT + HEALING_MACHINE_MODEL_NAME + "_blue.png"
    );
    ResourceLoadingService::GetInstance().LoadResource
    (
        ResourceLoadingService::RES_TEXTURES_ROOT + HEALING_MACHINE_MODEL_NAME + "_white.png"
    );
}

void PokeCenterHealingAnimationFlowState::SetCurrentStateDurationTimer() const
{
    auto& pokeCenterHealingAnimationState = mWorld.GetSingletonComponent<PokeCenterHealingAnimationStateSingletonComponent>();
    pokeCenterHealingAnimationState.mAnimationTimer = std::make_unique<Timer>(sHealingAnimationStateDurations.at(pokeCenterHealingAnimationState.mHealingAnimationStateQueue.front()));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

