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
#include "TurnOverEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/Timer.h"
#include "../utils/FileUtils.h"
#include "../utils/OSMessageBox.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../rendering/components/RenderableComponent.h"

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
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        if (encounterStateComponent.mWasPokemonCaught)
        {
            // CompleteAndTransitionTo<PokemonDescriptionPanel>();
        }
        else
        {
            encounterStateComponent.mPlayerChangedPokemonFromMainMenu         = false;
            encounterStateComponent.mIsOpponentsTurn                          = false;
            encounterStateComponent.mTurnsCompleted                           = 0;
            encounterStateComponent.mLastEncounterMainMenuActionSelected      = MainMenuActionType::ITEM;
            encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu = 0;
            CompleteAndTransitionTo<TurnOverEncounterFlowState>();
        }
    }
    else
    {
        UpdateCatchAttemptAnimation(dt);
    }
}

void BallAnimationEncounterFlowState::UpdateCatchAttemptAnimation(const float dt) const
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

        auto renderableComponent                    = std::make_unique<RenderableComponent>();
        renderableComponent->mTextureResourceId     = encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.front();
        renderableComponent->mActiveAnimationNameId = StringId("default");
        renderableComponent->mShaderNameId          = StringId("gui");
        renderableComponent->mAffectedByPerspective = false;

        const auto frameModelPath    = ResourceLoadingService::RES_MODELS_ROOT + BATTLE_ANIMATION_MODEL_FILE_NAME;
        auto& resourceLoadingService = ResourceLoadingService::GetInstance();
        renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(frameModelPath));

        encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.pop();

        auto transformComponent         = std::make_unique<TransformComponent>();
        transformComponent->mPosition.z = BATTLE_MOVE_ANIMATION_Z;
        transformComponent->mScale      = BATTLE_MOVE_SCALE;

        mWorld.AddComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(renderableComponent));
        mWorld.AddComponent<TransformComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(transformComponent));
    }
    else
    {
        DisplayCatchResultText();
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

void BallAnimationEncounterFlowState::DisplayCatchResultText() const
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    // Destroy ball usage chatbox
    DestroyActiveTextbox(mWorld);

    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    std::string catchResultText = "";

    if (encounterStateComponent.mWasPokemonCaught)
    {
        catchResultText += "All right!#" + encounterStateComponent.mOpponentPokemonRoster.at(0)->mName.GetString() + " was#caught!#@";
        catchResultText += "New POK^DEX data#will be addded for#" + encounterStateComponent.mOpponentPokemonRoster.at(0)->mName.GetString() + "!#+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == -1)
    {
        catchResultText += "The trainer#blocked the BALL!#@Don't be a thief!#+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 0)
    {
        catchResultText += "You missed the#POK^MON!#+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 1)
    {
        catchResultText += "Darn! The POK^MON#broke free!#+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 2)
    {
        catchResultText += "Aww! It appeared#to be caught!";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 3)
    {
        catchResultText += "Shoot! It was so#close too!#+END";
    }

    QueueDialogForChatbox(mainChatboxEntityId, catchResultText, mWorld);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
