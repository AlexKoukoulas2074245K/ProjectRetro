//
//  MoveAnimationEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MoveAnimationEncounterFlowState.h"
#include "MoveShakeEncounterFlowState.h"
#include "../components/TransformComponent.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../common/utils/FileUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../common/utils/OSMessageBox.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/TransitionAnimationStateSingletonComponent.h"
#include "../../overworld/systems/TransitionAnimationSystem.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/utils/Colors.h"
#include "../../rendering/utils/RenderingUtils.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../resources/TextureResource.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 MoveAnimationEncounterFlowState::PLAYER_POKEMON_SPRITE_START_POSITION   = glm::vec3(-0.39f, 0.06f, 0.1f);
const glm::vec3 MoveAnimationEncounterFlowState::PLAYER_POKEMON_SPRITE_END_POSITION     = glm::vec3(-1.0f, 0.06f, 0.1f);
const glm::vec3 MoveAnimationEncounterFlowState::OPPONENT_POKEMON_SPRITE_START_POSITION = glm::vec3(0.38f, 0.61f, 0.3f);
const glm::vec3 MoveAnimationEncounterFlowState::OPPONENT_POKEMON_SPRITE_END_POSITION   = glm::vec3(0.99f, 0.61f, 0.3f);

const std::string MoveAnimationEncounterFlowState::BATTLE_ANIMATION_MODEL_FILE_NAME = "battle_anim_quad.obj";
const std::string MoveAnimationEncounterFlowState::BATTLE_ANIMATION_DIR_NAME        = "battle_animations/";

const StringId MoveAnimationEncounterFlowState::DARK_FLIP_HUD_ELEMENTS_SPECIAL_CASE_SHADER_NAME = StringId("dark_flip_hud_special_case");
const StringId MoveAnimationEncounterFlowState::DEFAULT_GUI_SHADER_NAME                         = StringId("gui");

const float MoveAnimationEncounterFlowState::SPRITE_MOVEMENT_ANIMATION_SPEED = 2.0f;
const float MoveAnimationEncounterFlowState::BATTLE_MOVE_ANIMATION_Z         = -1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MoveAnimationEncounterFlowState::MoveAnimationEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (DoesMoveHaveSpeciallyHandledAnimation(encounterStateComponent.mLastMoveSelected))
    {                
        encounterStateComponent.mViewObjects.mBattleAnimationTimer = std::make_unique<Timer>
        (
            encounterStateComponent.mLastMoveSelected == StringId("CONFUSION") ? 
            TransitionAnimationSystem::WILD_FLASH_ANIMATION_STEP_DURATION : 
            BATTLE_SPECIAL_MOVE_ANIMATION_DELAY
        );

        encounterStateComponent.mSpecialMoveAnimationStep = 0;

        auto battleAnimationDirPath = ResourceLoadingService::RES_TEXTURES_ROOT + BATTLE_ANIMATION_DIR_NAME + encounterStateComponent.mLastMoveSelected.GetString() + "/";
        if (GetAllFilenamesInDirectory(battleAnimationDirPath).size() > 0)
        {
            LoadMoveAnimationFrames();
        }
    }
    else
    {
        LoadMoveAnimationFrames();
    }    
}

void MoveAnimationEncounterFlowState::VUpdate(const float dt)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (DoesMoveHaveSpeciallyHandledAnimation(encounterStateComponent.mLastMoveSelected))
    {
        UpdateSpeciallyHandledMoveAnimation(dt);
    }
    else
    {
        encounterStateComponent.mViewObjects.mBattleAnimationTimer->Update(dt);
        if (encounterStateComponent.mViewObjects.mBattleAnimationTimer->HasTicked())
        {
            encounterStateComponent.mViewObjects.mBattleAnimationTimer->Reset();        
            UpdateNormalFrameBasedMoveAnimation();
        }        
    }       
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void MoveAnimationEncounterFlowState::LoadMoveAnimationFrames() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& resourceLoadingService  = ResourceLoadingService::GetInstance();
    
    encounterStateComponent.mViewObjects.mBattleAnimationTimer = std::make_unique<Timer>(BATTLE_MOVE_ANIMATION_FRAME_DURATION);

    auto battleAnimationDirPath = ResourceLoadingService::RES_TEXTURES_ROOT + BATTLE_ANIMATION_DIR_NAME + encounterStateComponent.mLastMoveSelected.GetString() + "/";
    if (encounterStateComponent.mIsOpponentsTurn)
    {
        // This tests whether the move has a separate folder for the enemy animation as is in some cases (Gust, Growl, etc), 
        // and if so, loads these frames instead
        const auto testEnemyFramesPath = ResourceLoadingService::RES_TEXTURES_ROOT + BATTLE_ANIMATION_DIR_NAME + encounterStateComponent.mLastMoveSelected.GetString() + "_ENEMY/";
        const auto& testWhetherSeparateFoldersExistsResultFrames = GetAllFilenamesInDirectory(testEnemyFramesPath);
        
        if (testWhetherSeparateFoldersExistsResultFrames.size() > 0)
        {
            battleAnimationDirPath = testEnemyFramesPath;
        }
    }

    const auto& battleAnimFilenames = GetAllFilenamesInDirectory(battleAnimationDirPath);
    if (battleAnimFilenames.size() == 0)
    {
        ShowMessageBox
        (
            MessageBoxType::INFO,
            "Battle animation missing",
            "Battle animation frames for: " + encounterStateComponent.mLastMoveSelected.GetString() + " missing."
        );
        return;
    }
    
    for (const auto& filename : battleAnimFilenames)
    {
        encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.push
        (
            resourceLoadingService.LoadResource(battleAnimationDirPath + filename)
        );
    }        
}

void MoveAnimationEncounterFlowState::PrepareAllGuiSpritesForWhiteFlip() const
{    
    auto& encounterStateComponent  = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    auto& playerPokemonSpriteRenderableComponent = mWorld.GetComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);
    const auto& playerTextureResource = ResourceLoadingService::GetInstance().GetResource<TextureResource>(playerPokemonSpriteRenderableComponent.mTextureResourceId);

    glm::vec4 playerPrimaryLightColor, playerPrimaryDarkColor;
    GetPrimaryLightAndPrimaryDarkColorsFromSet(playerTextureResource.GetColorSet(), playerPrimaryLightColor, playerPrimaryDarkColor);

    playerPokemonSpriteRenderableComponent.mOverriddenLightColor = playerPrimaryLightColor;
    playerPokemonSpriteRenderableComponent.mOverriddenDarkColor  = playerPrimaryDarkColor;

    auto& opponentPokemonSpriteRenderableComponent = mWorld.GetComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
    const auto& opponentTextureResource = ResourceLoadingService::GetInstance().GetResource<TextureResource>(opponentPokemonSpriteRenderableComponent.mTextureResourceId);

    glm::vec4 opponentPrimaryLightColor, opponentPrimaryDarkColor;
    GetPrimaryLightAndPrimaryDarkColorsFromSet(opponentTextureResource.GetColorSet(), opponentPrimaryLightColor, opponentPrimaryDarkColor);

    opponentPokemonSpriteRenderableComponent.mOverriddenLightColor = opponentPrimaryLightColor;
    opponentPokemonSpriteRenderableComponent.mOverriddenDarkColor  = opponentPrimaryDarkColor;

    const auto& playerPokemonInfoTextboxEntities = GetAllTextboxResidentComponents(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, mWorld);
    for (const auto& entityId : playerPokemonInfoTextboxEntities)
    {
        auto& renderableComponent                 = mWorld.GetComponent<RenderableComponent>(entityId);
        renderableComponent.mOverriddenLightColor = playerPrimaryLightColor;
        renderableComponent.mOverriddenDarkColor  = playerPrimaryDarkColor;
    }
    auto& playerPokemonStatusDisplayRenderableComponent                 = mWorld.GetComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId);
    playerPokemonStatusDisplayRenderableComponent.mOverriddenLightColor = playerPrimaryLightColor;
    playerPokemonStatusDisplayRenderableComponent.mOverriddenDarkColor  = playerPrimaryDarkColor;

    const auto& opponentPokemonInfoTextboxEntities = GetAllTextboxResidentComponents(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, mWorld);
    for (const auto& entityId : opponentPokemonInfoTextboxEntities)
    {
        auto& renderableComponent                 = mWorld.GetComponent<RenderableComponent>(entityId);
        renderableComponent.mOverriddenLightColor = opponentPrimaryLightColor;
        renderableComponent.mOverriddenDarkColor  = opponentPrimaryDarkColor;
    }

    auto& opponentPokemonStatusDisplayRenderableComponent                 = mWorld.GetComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId);
    opponentPokemonStatusDisplayRenderableComponent.mOverriddenLightColor = opponentPrimaryLightColor;
    opponentPokemonStatusDisplayRenderableComponent.mOverriddenDarkColor  = opponentPrimaryDarkColor;

    const auto& mainChatboxEntities = GetAllTextboxResidentComponents(GetActiveTextboxEntityId(mWorld), mWorld);
    for (const auto& entityId : mainChatboxEntities)
    {
        if (mWorld.HasComponent<RenderableComponent>(entityId))
        {
            auto& renderableComponent                 = mWorld.GetComponent<RenderableComponent>(entityId);
            renderableComponent.mOverriddenLightColor = playerPrimaryLightColor;
            renderableComponent.mOverriddenDarkColor  = playerPrimaryDarkColor;
        }        
    }
}

void MoveAnimationEncounterFlowState::UpdateSpeciallyHandledMoveAnimation(const float dt)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
        
    // Self-time managed special attack animations
    if (encounterStateComponent.mLastMoveSelected == StringId("QUICK_ATTACK"))
    {
        UpdateQuickAttackAnimation(dt);
    }
    else
    {
        encounterStateComponent.mViewObjects.mBattleAnimationTimer->Update(dt);
        if (encounterStateComponent.mViewObjects.mBattleAnimationTimer->HasTicked())
        {
            encounterStateComponent.mViewObjects.mBattleAnimationTimer->Reset();

            if (encounterStateComponent.mLastMoveSelected == StringId("CONFUSION"))
            {
                UpdateConfusionAnimation();
            }
            else if (encounterStateComponent.mLastMoveSelected == StringId("HARDEN"))
            {
                UpdateHardenAnimation();
            }
            else if (encounterStateComponent.mLastMoveSelected == StringId("LEER"))
            {
                UpdateLeerAnimation();
            }
            else if (encounterStateComponent.mLastMoveSelected == StringId("TACKLE"))
            {
                UpdateTackleAnimation();
            }
            else if (encounterStateComponent.mLastMoveSelected == StringId("TAIL_WHIP"))
            {
                UpdateTailWhipAnimation();
            }
        }
    }
}

void MoveAnimationEncounterFlowState::UpdateNormalFrameBasedMoveAnimation()
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId != ecs::NULL_ENTITY_ID)
    {
        mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId);
        encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = ecs::NULL_ENTITY_ID;
    }

    if (encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.size() > 0)
    {
        encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = mWorld.CreateEntity();

        auto renderableComponent = std::make_unique<RenderableComponent>();
        renderableComponent->mTextureResourceId = encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.front();
        renderableComponent->mActiveAnimationNameId = StringId("default");
        renderableComponent->mShaderNameId = StringId("gui_unaffected_by_white_flip");
        renderableComponent->mAffectedByPerspective = false;

        const auto frameModelPath = ResourceLoadingService::RES_MODELS_ROOT + BATTLE_ANIMATION_MODEL_FILE_NAME;
        auto& resourceLoadingService = ResourceLoadingService::GetInstance();
        renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(frameModelPath));

        encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.pop();

        auto transformComponent = std::make_unique<TransformComponent>();
        transformComponent->mPosition.z = BATTLE_MOVE_ANIMATION_Z;

        const auto testEnemyFramesPath = ResourceLoadingService::RES_TEXTURES_ROOT + BATTLE_ANIMATION_DIR_NAME + encounterStateComponent.mLastMoveSelected.GetString() + "_ENEMY/";
        const auto& testWhetherSeparateFoldersExistsResultFrames = GetAllFilenamesInDirectory(testEnemyFramesPath);

        // In case we loaded enemy specific frames, position and scale them normally
        if (testWhetherSeparateFoldersExistsResultFrames.size() > 0)
        {
            transformComponent->mScale = glm::vec3(2.0f, 2.0f, 2.0f);
            transformComponent->mPosition.y = 0.0f;
        }
        // Otherwise, position them normally for player moves, and flip them horizontally and position them appropriately 
        // for enemy ones
        else
        {
            transformComponent->mScale = glm::vec3(encounterStateComponent.mIsOpponentsTurn ? -2.0f : 2.0f, 2.0f, 2.0f);
            transformComponent->mPosition.y = encounterStateComponent.mIsOpponentsTurn ? -0.55f : 0.0f;
        }

        mWorld.AddComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(renderableComponent));
        mWorld.AddComponent<TransformComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(transformComponent));
    }
    else
    {
        CompleteAndTransitionTo<MoveShakeEncounterFlowState>();
    }
}

void MoveAnimationEncounterFlowState::UpdateConfusionAnimation()
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& transitionStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();

    switch (encounterStateComponent.mSpecialMoveAnimationStep)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        {            
            transitionStateComponent.mAnimationProgressionStep = encounterStateComponent.mSpecialMoveAnimationStep;
            ++encounterStateComponent.mSpecialMoveAnimationStep;
        } break;

        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        {            
            transitionStateComponent.mAnimationProgressionStep = 6 - encounterStateComponent.mSpecialMoveAnimationStep;
            ++encounterStateComponent.mSpecialMoveAnimationStep;
        } break;
        
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        {
            transitionStateComponent.mAnimationProgressionStep = -12 + encounterStateComponent.mSpecialMoveAnimationStep;
            ++encounterStateComponent.mSpecialMoveAnimationStep;
        } break;

        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        {
            transitionStateComponent.mAnimationProgressionStep = 18 - encounterStateComponent.mSpecialMoveAnimationStep;
            ++encounterStateComponent.mSpecialMoveAnimationStep;
        } break;

        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        {
            transitionStateComponent.mAnimationProgressionStep = -24 + encounterStateComponent.mSpecialMoveAnimationStep;
            ++encounterStateComponent.mSpecialMoveAnimationStep;
        } break;

        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        {
            transitionStateComponent.mAnimationProgressionStep = 30 - encounterStateComponent.mSpecialMoveAnimationStep;
            ++encounterStateComponent.mSpecialMoveAnimationStep;
        } break;

        case 34:
        case 35:
        case 36:
        {
            transitionStateComponent.mAnimationProgressionStep = -36 + encounterStateComponent.mSpecialMoveAnimationStep;
            ++encounterStateComponent.mSpecialMoveAnimationStep;
        }
    }              
}

void MoveAnimationEncounterFlowState::UpdateHardenAnimation()
{   
    auto& encounterStateComponent  = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& transitionStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();

    switch (encounterStateComponent.mSpecialMoveAnimationStep)
    {
        case 0:
        {
            PrepareAllGuiSpritesForWhiteFlip();
            transitionStateComponent.mWhiteFlipProgressionStep = 1;
            encounterStateComponent.mSpecialMoveAnimationStep++;
        } break;

        case 1:
        {
            UpdateNormalFrameBasedMoveAnimation();

            if (encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.size() == 0)
            {
                encounterStateComponent.mSpecialMoveAnimationStep++;
            }
        } break;
        case 2:
        {
            if (encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId != ecs::NULL_ENTITY_ID)
            {
                mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId);
                encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = ecs::NULL_ENTITY_ID;
            }

            encounterStateComponent.mSpecialMoveAnimationStep++;
        } break;

        case 3:
        case 4:
        {
            transitionStateComponent.mWhiteFlipProgressionStep = 2;
            encounterStateComponent.mSpecialMoveAnimationStep++;
        } break;

        case 5:
        case 6:
        {
            transitionStateComponent.mWhiteFlipProgressionStep = 3;
            encounterStateComponent.mSpecialMoveAnimationStep++;
        } break;

        case 7:
        {
            transitionStateComponent.mWhiteFlipProgressionStep = 1;
            encounterStateComponent.mSpecialMoveAnimationStep++;
        } break;

        case 8:
        {
            transitionStateComponent.mWhiteFlipProgressionStep = 0;
            encounterStateComponent.mSpecialMoveAnimationStep  = 0;
            CompleteAndTransitionTo<MoveShakeEncounterFlowState>();
        } break;
    }    
}

void MoveAnimationEncounterFlowState::UpdateLeerAnimation()
{
    auto& encounterStateComponent  = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& transitionStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    
    switch (encounterStateComponent.mSpecialMoveAnimationStep)
    {
        case 0:
        case 3:
        case 6:
        {
            encounterStateComponent.mSpecialMoveAnimationStep++;
            transitionStateComponent.mDarkFlipProgressionStep = 1;
            
            auto& playerPokemonSpriteRenderableComponent   = mWorld.GetComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);
            auto& opponentPokemonSpriteRenderableComponent = mWorld.GetComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);

            // The active player, opponent and chatbox sprites during dark flip animations change their colors differently 
            // from other HUD elements.
            playerPokemonSpriteRenderableComponent.mShaderNameId   = DARK_FLIP_HUD_ELEMENTS_SPECIAL_CASE_SHADER_NAME;            
            opponentPokemonSpriteRenderableComponent.mShaderNameId = DARK_FLIP_HUD_ELEMENTS_SPECIAL_CASE_SHADER_NAME;            

        } break;

        case 1:    
        case 4:
        {
            encounterStateComponent.mSpecialMoveAnimationStep++;
            transitionStateComponent.mDarkFlipProgressionStep = 2;
        } break;

        case 2:
        case 5:
        {
            encounterStateComponent.mSpecialMoveAnimationStep++;
            transitionStateComponent.mDarkFlipProgressionStep = 3;
        } break;

        case 7:
        {
            CompleteAndTransitionTo<MoveShakeEncounterFlowState>();
            transitionStateComponent.mDarkFlipProgressionStep = 0;

            auto& playerPokemonSpriteRenderableComponent   = mWorld.GetComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);
            auto& opponentPokemonSpriteRenderableComponent = mWorld.GetComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);

            playerPokemonSpriteRenderableComponent.mShaderNameId   = DEFAULT_GUI_SHADER_NAME;
            opponentPokemonSpriteRenderableComponent.mShaderNameId = DEFAULT_GUI_SHADER_NAME;

            encounterStateComponent.mSpecialMoveAnimationStep      = 0;
        } break;
    }
}

void MoveAnimationEncounterFlowState::UpdateQuickAttackAnimation(const float dt)
{
    auto& encounterStateComponent       = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto attackingPokemonSpriteEntityId = encounterStateComponent.mIsOpponentsTurn ? encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId : encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId;
    auto& attackerTransformComponent    = mWorld.GetComponent<TransformComponent>(attackingPokemonSpriteEntityId);

    switch (encounterStateComponent.mSpecialMoveAnimationStep)
    {
        case 0:
        {
            attackerTransformComponent.mPosition.x += 
                encounterStateComponent.mIsOpponentsTurn ? 
                SPRITE_MOVEMENT_ANIMATION_SPEED * dt :
                -SPRITE_MOVEMENT_ANIMATION_SPEED * dt;

            if 
            (
                encounterStateComponent.mIsOpponentsTurn && 
                attackerTransformComponent.mPosition.x > OPPONENT_POKEMON_SPRITE_END_POSITION.x
            )
            {
                attackerTransformComponent.mPosition.x = OPPONENT_POKEMON_SPRITE_END_POSITION.x;
                encounterStateComponent.mSpecialMoveAnimationStep++;
            }
            else if 
            (
                encounterStateComponent.mIsOpponentsTurn == false &&
                attackerTransformComponent.mPosition.x < PLAYER_POKEMON_SPRITE_END_POSITION.x
            )
            {
                attackerTransformComponent.mPosition.x = PLAYER_POKEMON_SPRITE_END_POSITION.x;
                encounterStateComponent.mSpecialMoveAnimationStep++;
            }
        } break;

        case 1:
        {
            encounterStateComponent.mViewObjects.mBattleAnimationTimer->Update(dt);
            if (encounterStateComponent.mViewObjects.mBattleAnimationTimer->HasTicked())
            {
                encounterStateComponent.mViewObjects.mBattleAnimationTimer->Reset();

                UpdateNormalFrameBasedMoveAnimation();

                if (encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.size() == 0)
                {
                    encounterStateComponent.mSpecialMoveAnimationStep++;
                }
            }
            
        } break;

        case 2:
        {
            if (encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId != ecs::NULL_ENTITY_ID)
            {
                mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId);
                encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = ecs::NULL_ENTITY_ID;
            }

            attackerTransformComponent.mPosition.x = 
                encounterStateComponent.mIsOpponentsTurn ? 
                OPPONENT_POKEMON_SPRITE_START_POSITION.x : 
                PLAYER_POKEMON_SPRITE_START_POSITION.x;

            CompleteAndTransitionTo<MoveShakeEncounterFlowState>();
        } break;
    }
}

void MoveAnimationEncounterFlowState::UpdateTackleAnimation()
{
    auto& encounterStateComponent       = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto attackingPokemonSpriteEntityId = encounterStateComponent.mIsOpponentsTurn ? encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId : encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId;
    auto& attackerTransformComponent    = mWorld.GetComponent<TransformComponent>(attackingPokemonSpriteEntityId);

    switch (encounterStateComponent.mSpecialMoveAnimationStep)
    {
        case 0:
        {
            attackerTransformComponent.mPosition.x += encounterStateComponent.mIsOpponentsTurn ? -GUI_PIXEL_SIZE * 8 : GUI_PIXEL_SIZE * 8;
            encounterStateComponent.mSpecialMoveAnimationStep++;
        } break;            

        case 1:
        {
            attackerTransformComponent.mPosition.x -= encounterStateComponent.mIsOpponentsTurn ? -GUI_PIXEL_SIZE * 8 : GUI_PIXEL_SIZE * 8;
            encounterStateComponent.mSpecialMoveAnimationStep++;
        } break;

        case 2:
        {
            CompleteAndTransitionTo<MoveShakeEncounterFlowState>();
            encounterStateComponent.mSpecialMoveAnimationStep = 0;
        } break;
    }
}

void MoveAnimationEncounterFlowState::UpdateTailWhipAnimation()
{
    auto& encounterStateComponent       = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto attackingPokemonSpriteEntityId = encounterStateComponent.mIsOpponentsTurn ? encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId : encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId;
    auto& attackerTransformComponent    = mWorld.GetComponent<TransformComponent>(attackingPokemonSpriteEntityId);

    switch (encounterStateComponent.mSpecialMoveAnimationStep)
    {
        case 0:
        {
            attackerTransformComponent.mPosition.x += encounterStateComponent.mIsOpponentsTurn ? -GUI_PIXEL_SIZE * 8 : GUI_PIXEL_SIZE * 8;
            encounterStateComponent.mSpecialMoveAnimationStep++;
        } break;            

        case 1:
        {
            attackerTransformComponent.mPosition.x -= encounterStateComponent.mIsOpponentsTurn ? -GUI_PIXEL_SIZE * 8 : GUI_PIXEL_SIZE * 8;
            encounterStateComponent.mSpecialMoveAnimationStep++;
        } break;

        case 2:
        {
            attackerTransformComponent.mPosition.x += encounterStateComponent.mIsOpponentsTurn ? -GUI_PIXEL_SIZE * 8 : GUI_PIXEL_SIZE * 8;
            encounterStateComponent.mSpecialMoveAnimationStep++;
        } break;

        case 3:
        {
            attackerTransformComponent.mPosition.x -= encounterStateComponent.mIsOpponentsTurn ? -GUI_PIXEL_SIZE * 8 : GUI_PIXEL_SIZE * 8;
            encounterStateComponent.mSpecialMoveAnimationStep++;
        } break;

        case 4:
        {
            encounterStateComponent.mSpecialMoveAnimationStep = 0;
            CompleteAndTransitionTo<MoveShakeEncounterFlowState>();
        } break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
