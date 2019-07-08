//
//  PlayerPokemonSummonEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PlayerPokemonSummonEncounterFlowState.h"
#include "MainMenuEncounterFlowState.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/utils/FileUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"
#include "../components/TransformComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PlayerPokemonSummonEncounterFlowState::PLAYER_STATUS_DISPLAY_POSITION       = glm::vec3(0.3568f, -0.08f, 0.0f);
const glm::vec3 PlayerPokemonSummonEncounterFlowState::PLAYER_HEALTHBAR_DISPLAY_POSITION    = glm::vec3(0.3568f, -0.08f, 0.1f);
const glm::vec3 PlayerPokemonSummonEncounterFlowState::PLAYER_POKEMON_SPRITE_START_POSITION = glm::vec3(-1.0f, 0.06f, 0.1f);
const glm::vec3 PlayerPokemonSummonEncounterFlowState::PLAYER_POKEMON_SPRITE_END_POSITION   = glm::vec3(-0.39f, 0.06f, 0.0f);
const glm::vec3 PlayerPokemonSummonEncounterFlowState::PLAYER_STATUS_DISPLAY_SCALE          = glm::vec3(1.0f, 1.0f, 1.0f);
const glm::vec3 PlayerPokemonSummonEncounterFlowState::SPRITE_SCALE                         = glm::vec3(0.49f, 0.49f, 1.0f);

const std::string PlayerPokemonSummonEncounterFlowState::POKEMON_SUMMON_BATTLE_ANIM_MODEL_FILE_NAME = "battle_anim_quad.obj";
const std::string PlayerPokemonSummonEncounterFlowState::POKEMON_SUMMON_BATTLE_ANIMATION_DIR_NAME   = "battle_animations/player_pokemon_summon/";

const float PlayerPokemonSummonEncounterFlowState::SPRITE_ANIMATION_SPEED = 2.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerPokemonSummonEncounterFlowState::PlayerPokemonSummonEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{    
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);

    // Player pokemon status display
    encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId = LoadAndCreatePlayerPokemonStatusDisplay
    (
        PLAYER_STATUS_DISPLAY_POSITION,
        PLAYER_STATUS_DISPLAY_SCALE,
        mWorld
    );
    
    //TODO: select appropriate bar color 
    encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId = LoadAndCreatePokemonHealthBar
    (
        PokemonHealthBarStatus::GREEN,
        PLAYER_HEALTHBAR_DISPLAY_POSITION,
        PLAYER_STATUS_DISPLAY_SCALE,
        mWorld
    );

    //TODO: displace health bar appropriately

    // Write player's pokemon name
    WriteTextAtTextboxCoords
    (
        encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
        playerStateComponent.mPlayerPokemonRoster.front().mName.GetString(),
        0,
        0,
        mWorld
    );
    
    // Write player's pokemon level
    WriteTextAtTextboxCoords
    (
        encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
        "=" + std::to_string(playerStateComponent.mPlayerPokemonRoster.front().mLevel),
        4,
        1,
        mWorld
    );
    
    const auto playersTotalHp = 31;
    const auto playersCurrentHp = 15;
    
    // Write player's pokemon current hp
    WriteTextAtTextboxCoords
    (
        encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
        std::to_string(playersCurrentHp) + "/",
        4 - static_cast<int>(std::to_string(playersCurrentHp).size()),
        3,
        mWorld
    );
    
    // Write player's pokemon current hp
    WriteTextAtTextboxCoords
    (
        encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
        std::to_string(playersTotalHp),
        8 - static_cast<int>(std::to_string(playersTotalHp).size()),
        3,
        mWorld
    );

    // Pikachu gets summoned from the side of the screen, while all other pokemon via pokeball summons
    if (playerStateComponent.mPlayerPokemonRoster.front().mName == StringId("PIKACHU"))
    { 
        encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId = ecs::NULL_ENTITY_ID;
    }
    else
    {
        LoadPokemonSummonBattleAnimation();
    }
}

void PlayerPokemonSummonEncounterFlowState::VUpdate(const float dt)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    

    // Standard pokemon summon animation flow
    if (playerStateComponent.mPlayerPokemonRoster.front().mName != StringId("PIKACHU"))
    {
        encounterStateComponent.mViewObjects.mBattleAnimationTimer->Update(dt);
        if (encounterStateComponent.mViewObjects.mBattleAnimationTimer->HasTicked())
        {
            encounterStateComponent.mViewObjects.mBattleAnimationTimer->Reset();
            if (encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId != ecs::NULL_ENTITY_ID)
            {            
                mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId);
                encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = ecs::NULL_ENTITY_ID;
            }
        
            if (encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.size() > 0)
            {
                encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = mWorld.CreateEntity();

                auto renderableComponent = std::make_unique<RenderableComponent>();
                renderableComponent->mTextureResourceId     = encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.front();
                renderableComponent->mActiveAnimationNameId = StringId("default");
                renderableComponent->mShaderNameId          = StringId("gui");
                renderableComponent->mAffectedByPerspective = false;

                const auto frameModelPath = ResourceLoadingService::RES_MODELS_ROOT + POKEMON_SUMMON_BATTLE_ANIM_MODEL_FILE_NAME;
                auto& resourceLoadingService = ResourceLoadingService::GetInstance();
                renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(frameModelPath));

                encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.pop();

                auto transformComponent = std::make_unique<TransformComponent>();
                transformComponent->mPosition.z = -1.0f;
                transformComponent->mScale = glm::vec3(2.0f, 2.0f, 2.0f);

                mWorld.AddComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(renderableComponent));
                mWorld.AddComponent<TransformComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(transformComponent));
            }
            else
            {                
                encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId = LoadAndCreatePokemonSprite
                (
                    playerStateComponent.mPlayerPokemonRoster.front().mName,
                    false,
                    PLAYER_POKEMON_SPRITE_END_POSITION,
                    SPRITE_SCALE,
                    mWorld
                );

                DestroyActiveTextbox(mWorld);
                CompleteAndTransitionTo<MainMenuEncounterFlowState>();
            }
        }
    }
    // Pikachu flow
    else
    {
        if (encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId == ecs::NULL_ENTITY_ID)
        {
            encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId = LoadAndCreatePokemonSprite
            (
                playerStateComponent.mPlayerPokemonRoster.front().mName,
                false,
                PLAYER_POKEMON_SPRITE_START_POSITION,
                SPRITE_SCALE,
                mWorld
            );
        }
        else
        {
            auto& playerTrainerSpriteTransformComponent = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);

            playerTrainerSpriteTransformComponent.mPosition.x += SPRITE_ANIMATION_SPEED * dt;
            if (playerTrainerSpriteTransformComponent.mPosition.x > PLAYER_POKEMON_SPRITE_END_POSITION.x)
            {
                playerTrainerSpriteTransformComponent.mPosition.x = PLAYER_POKEMON_SPRITE_END_POSITION.x;                
                DestroyActiveTextbox(mWorld);
                CompleteAndTransitionTo<MainMenuEncounterFlowState>();
            }
        }        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PlayerPokemonSummonEncounterFlowState::LoadPokemonSummonBattleAnimation() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& resourceLoadingService  = ResourceLoadingService::GetInstance();
    
    const auto transitionAnimationDirPath = ResourceLoadingService::RES_TEXTURES_ROOT + POKEMON_SUMMON_BATTLE_ANIMATION_DIR_NAME;
    const auto& encounterAnimFilenames    = GetAllFilenamesInDirectory(transitionAnimationDirPath);
    for (const auto& filename : encounterAnimFilenames)
    {
        encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.push
        (
            resourceLoadingService.LoadResource(transitionAnimationDirPath + filename)
        );
    }

    encounterStateComponent.mViewObjects.mBattleAnimationTimer = std::make_unique<Timer>(BATTLE_ANIMATION_FRAME_DURATION);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

