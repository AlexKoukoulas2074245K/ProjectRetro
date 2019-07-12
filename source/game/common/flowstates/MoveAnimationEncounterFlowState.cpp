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
#include "MoveOpponentShakeEncounterFlowState.h"
#include "../components/TransformComponent.h"
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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MoveAnimationEncounterFlowState::MoveAnimationEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    LoadMoveAnimationFrames();
}

void MoveAnimationEncounterFlowState::VUpdate(const float dt)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

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
            renderableComponent->mTextureResourceId = encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.front();
            renderableComponent->mActiveAnimationNameId = StringId("default");
            renderableComponent->mShaderNameId = StringId("gui");
            renderableComponent->mAffectedByPerspective = false;

            const auto frameModelPath = ResourceLoadingService::RES_MODELS_ROOT + BATTLE_ANIMATION_MODEL_FILE_NAME;
            auto& resourceLoadingService = ResourceLoadingService::GetInstance();
            renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(frameModelPath));

            encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.pop();

            auto transformComponent = std::make_unique<TransformComponent>();
            
            transformComponent->mPosition.z = -1.0f;
            transformComponent->mScale      = glm::vec3(encounterStateComponent.mIsOpponentsTurn ? -2.0f : 2.0f, 2.0f, 2.0f);
            transformComponent->mPosition.y = encounterStateComponent.mIsOpponentsTurn ? -0.55f : 0.0f;
            
            mWorld.AddComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(renderableComponent));
            mWorld.AddComponent<TransformComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(transformComponent));
        }
        else
        {
            CompleteAndTransitionTo<MoveOpponentShakeEncounterFlowState>();
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
    
    encounterStateComponent.mViewObjects.mBattleAnimationTimer = std::make_unique<Timer>(BATTLE_ANIMATION_FRAME_DURATION);

    const auto battleAnimationDirPath = ResourceLoadingService::RES_TEXTURES_ROOT + BATTLE_ANIMATION_DIR_NAME + encounterStateComponent.mLastMoveSelected.GetString() + "/";
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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
