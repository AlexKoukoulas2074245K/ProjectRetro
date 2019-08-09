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
#include "../../common/utils/OSMessageBox.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string MoveAnimationEncounterFlowState::BATTLE_ANIMATION_MODEL_FILE_NAME = "battle_anim_quad.obj";
const std::string MoveAnimationEncounterFlowState::BATTLE_ANIMATION_DIR_NAME = "battle_animations/";

const float MoveAnimationEncounterFlowState::BATTLE_MOVE_ANIMATION_Z = -1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MoveAnimationEncounterFlowState::MoveAnimationEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (DoesMoveHaveSpeciallyHandledAnimation(encounterStateComponent.mLastMoveSelected))
    {
        encounterStateComponent.mViewObjects.mBattleAnimationTimer = std::make_unique<Timer>(BATTLE_SPECIAL_MOVE_ANIMATION_DELAY);
        encounterStateComponent.mSpecialMoveAnimationStep = 0;
    }
    else
    {
        LoadMoveAnimationFrames();
    }    
}

void MoveAnimationEncounterFlowState::VUpdate(const float dt)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    encounterStateComponent.mViewObjects.mBattleAnimationTimer->Update(dt);
    if (encounterStateComponent.mViewObjects.mBattleAnimationTimer->HasTicked())
    {
        encounterStateComponent.mViewObjects.mBattleAnimationTimer->Reset();

        if (DoesMoveHaveSpeciallyHandledAnimation(encounterStateComponent.mLastMoveSelected))
        {
            UpdateSpeciallyHandledMoveAnimation();
        }
        else
        {
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

void MoveAnimationEncounterFlowState::UpdateSpeciallyHandledMoveAnimation()
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    if (encounterStateComponent.mLastMoveSelected == StringId("TACKLE"))
    {
        UpdateTackleAnimation();
    }    
    else if (encounterStateComponent.mLastMoveSelected == StringId("TAIL_WHIP"))
    {
        UpdateTailWhipAnimation();
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
        renderableComponent->mShaderNameId = StringId("gui");
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
