//
//  FastAsleepAnimationEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 20/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "FastAsleepAnimationEncounterFlowState.h"
#include "TurnOverEncounterFlowState.h"
#include "../components/TransformComponent.h"
#include "../utils/FileUtils.h"
#include "../utils/OSMessageBox.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 FastAsleepAnimationEncounterFlowState::FAST_ASLEEP_ANIMATION_SCALE = glm::vec3(2.0f, 2.0f, 1.0f);

const std::string FastAsleepAnimationEncounterFlowState::BATTLE_ANIMATION_MODEL_FILE_NAME = "battle_anim_quad.obj";
const std::string FastAsleepAnimationEncounterFlowState::PLAYER_FAST_ASLEEP_ANIMATION_DIR = "battle_animations/FAST_ASLEEP/";
const std::string FastAsleepAnimationEncounterFlowState::ENEMY_FAST_ASLEEP_ANIMATION_DIR  = "battle_animations/FAST_ASLEEP_ENEMY/";
const std::string FastAsleepAnimationEncounterFlowState::FAST_ASLEEP_SFX_NAME             = "encounter/sleeping";

const float FastAsleepAnimationEncounterFlowState::FAST_ASLEEP_ANIMATION_Z = -1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

FastAsleepAnimationEncounterFlowState::FastAsleepAnimationEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    LoadFastAsleepAnimationFrames();
}

void FastAsleepAnimationEncounterFlowState::VUpdate(const float dt)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    encounterStateComponent.mViewObjects.mBattleAnimationTimer->Update(dt);
    if (encounterStateComponent.mViewObjects.mBattleAnimationTimer->HasTicked())
    {
        encounterStateComponent.mViewObjects.mBattleAnimationTimer->Reset();
        
        if (encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId != ecs::NULL_ENTITY_ID)
        {
            mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId);
            encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = ecs::NULL_ENTITY_ID;
        }
        
        if (encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.size() > 0)
        {
            encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = mWorld.CreateEntity();
            
            auto renderableComponent                    = std::make_unique<RenderableComponent>();
            renderableComponent->mTextureResourceId     = encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.front();
            renderableComponent->mActiveAnimationNameId = StringId("default");
            renderableComponent->mShaderNameId          = StringId("gui_unaffected_by_white_flip");
            renderableComponent->mAffectedByPerspective = false;
            
            const auto frameModelPath    = ResourceLoadingService::RES_MODELS_ROOT + BATTLE_ANIMATION_MODEL_FILE_NAME;
            auto& resourceLoadingService = ResourceLoadingService::GetInstance();
            renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(frameModelPath));
            
            encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.pop();
            
            auto transformComponent         = std::make_unique<TransformComponent>();
            transformComponent->mPosition.z = FAST_ASLEEP_ANIMATION_Z;
            transformComponent->mScale      = FAST_ASLEEP_ANIMATION_SCALE;
            
            mWorld.AddComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(renderableComponent));
            mWorld.AddComponent<TransformComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(transformComponent));
        }
        else
        {
            DestroyActiveTextbox(mWorld);
            CompleteAndTransitionTo<TurnOverEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void FastAsleepAnimationEncounterFlowState::LoadFastAsleepAnimationFrames() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& resourceLoadingService  = ResourceLoadingService::GetInstance();
    
    encounterStateComponent.mViewObjects.mBattleAnimationTimer = std::make_unique<Timer>(BATTLE_MOVE_ANIMATION_FRAME_DURATION);
    
    auto battleAnimationDirPath = ResourceLoadingService::RES_TEXTURES_ROOT +
    (
        encounterStateComponent.mIsOpponentsTurn ?
        ENEMY_FAST_ASLEEP_ANIMATION_DIR :
        PLAYER_FAST_ASLEEP_ANIMATION_DIR
    );
    
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
    
    SoundService::GetInstance().PlaySfx(FAST_ASLEEP_SFX_NAME);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

