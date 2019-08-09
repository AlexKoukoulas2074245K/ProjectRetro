//
//  BallAnimationEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 09/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BallAnimationEncounterFlowState.h"
#include "BallUsageResultTextEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/Timer.h"
#include "../utils/FileUtils.h"
#include "../utils/OSMessageBox.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/utils/RenderingUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string BallAnimationEncounterFlowState::BATTLE_ANIMATION_MODEL_FILE_NAME = "battle_anim_quad.obj";
const std::string BallAnimationEncounterFlowState::BATTLE_ANIMATION_DIR_NAME        = "battle_animations/";

const glm::vec3 BallAnimationEncounterFlowState::BATTLE_MOVE_SCALE = glm::vec3(2.0f, 2.0f, 1.0f);

const float BallAnimationEncounterFlowState::BATTLE_MOVE_ANIMATION_Z = -1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

BallAnimationEncounterFlowState::BallAnimationEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    LoadCatchAttemptFrames();
}

void BallAnimationEncounterFlowState::VUpdate(const float dt)
{    
    UpdateCatchAttemptAnimation(dt);    
}

void BallAnimationEncounterFlowState::UpdateCatchAttemptAnimation(const float dt)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    encounterStateComponent.mViewObjects.mBattleAnimationTimer->Update(dt);
    if (encounterStateComponent.mViewObjects.mBattleAnimationTimer->HasTicked())
    {
        encounterStateComponent.mViewObjects.mBattleAnimationTimer->Reset();

        if (encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId != ecs::NULL_ENTITY_ID)
        {
            if (encounterStateComponent.mWasPokemonCaught == false || encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.size() != 0)
            {            
                mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId);
                encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = ecs::NULL_ENTITY_ID;
            }            
        }

        if (encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.size() > 0)
        {
            encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = mWorld.CreateEntity();

            auto renderableComponent                    = std::make_unique<RenderableComponent>();
            renderableComponent->mTextureResourceId     = encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.front();
            renderableComponent->mActiveAnimationNameId = StringId("default");
            renderableComponent->mShaderNameId          = StringId("gui");
            renderableComponent->mAffectedByPerspective = false;
        
            const auto frameModelPath    = ResourceLoadingService::RES_MODELS_ROOT + BATTLE_ANIMATION_MODEL_FILE_NAME;
            auto& resourceLoadingService = ResourceLoadingService::GetInstance();
            renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(frameModelPath));
            
            auto transformComponent         = std::make_unique<TransformComponent>();
            transformComponent->mPosition.z = BATTLE_MOVE_ANIMATION_Z;
            transformComponent->mScale      = BATTLE_MOVE_SCALE;

            mWorld.AddComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(renderableComponent));
            mWorld.AddComponent<TransformComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(transformComponent));

            // Change last frame of pokemon caught animation to reflect caught pokemon's color palette
            if (encounterStateComponent.mWasPokemonCaught && encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.size() == 1)
            {
                OverrideEntityPrimaryColorsBasedOnAnotherEntityPrimaryColors
                (
                    encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId,
                    encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId,
                    mWorld
                );
            }

            encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.pop();
        }
        else
        {
            CompleteAndTransitionTo<BallUsageResultTextEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void BallAnimationEncounterFlowState::LoadCatchAttemptFrames() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    encounterStateComponent.mViewObjects.mBattleAnimationTimer = std::make_unique<Timer>(BALL_USAGE_ANIMATION_FRAME_DURATION);

    auto battleAnimationDirPath = ResourceLoadingService::RES_TEXTURES_ROOT + BATTLE_ANIMATION_DIR_NAME;    
    
    if (encounterStateComponent.mWasPokemonCaught)
    {
        battleAnimationDirPath += "POKEMON_CAUGHT/";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == -1)
    {
        battleAnimationDirPath += "POKEMON_BROKE_FREE_THIEF/";
    }
    else 
    {
        battleAnimationDirPath += "POKEMON_BROKE_FREE_" + std::to_string(encounterStateComponent.mBallThrownShakeCount) + "/";
    }
    
    const auto& battleAnimFilenames = GetAllFilenamesInDirectory(battleAnimationDirPath);
    if (battleAnimFilenames.size() == 0)
    {
        ShowMessageBox
        (
            MessageBoxType::INFO,
            "Ball usage animation missing",
            "Ball usage animation frames for: " + battleAnimationDirPath + " missing."
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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
