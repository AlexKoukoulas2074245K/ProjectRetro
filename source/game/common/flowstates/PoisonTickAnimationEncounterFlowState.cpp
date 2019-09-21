//
//  PoisonTickAnimationEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 19/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "HealthDepletionEncounterFlowState.h"
#include "PoisonTickAnimationEncounterFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/FileUtils.h"
#include "../utils/PokemonMoveUtils.h"
#include "../utils/OSMessageBox.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PoisonTickAnimationEncounterFlowState::POISON_TICK_ANIMATION_SCALE = glm::vec3(2.0f, 2.0f, 1.0f);

const std::string PoisonTickAnimationEncounterFlowState::BATTLE_ANIMATION_MODEL_FILE_NAME = "battle_anim_quad.obj";
const std::string PoisonTickAnimationEncounterFlowState::PLAYER_POISON_TICK_ANIMATION_DIR = "battle_animations/POISON_TICK/";
const std::string PoisonTickAnimationEncounterFlowState::ENEMY_POISON_TICK_ANIMATION_DIR  = "battle_animations/POISON_TICK_ENEMY/";
const std::string PoisonTickAnimationEncounterFlowState::POISON_TICK_SFX_NAME             = "encounter/poison_tick";

const float PoisonTickAnimationEncounterFlowState::POISON_TICK_ANIMATION_Z = -1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PoisonTickAnimationEncounterFlowState::PoisonTickAnimationEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{    
    LoadPoisonTickFrames();
}

void PoisonTickAnimationEncounterFlowState::VUpdate(const float dt)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    auto& attackingPokemon =
        encounterStateComponent.mIsOpponentsTurn ?
        *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex] :
        *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];

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

            auto renderableComponent = std::make_unique<RenderableComponent>();
            renderableComponent->mTextureResourceId = encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.front();
            renderableComponent->mActiveAnimationNameId = StringId("default");
            renderableComponent->mShaderNameId = StringId("gui_unaffected_by_white_flip");
            renderableComponent->mAffectedByPerspective = false;

            const auto frameModelPath = ResourceLoadingService::RES_MODELS_ROOT + BATTLE_ANIMATION_MODEL_FILE_NAME;
            auto& resourceLoadingService = ResourceLoadingService::GetInstance();
            renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(frameModelPath));

            encounterStateComponent.mViewObjects.mBattleAnimationFrameResourceIdQueue.pop();

            auto transformComponent         = std::make_unique<TransformComponent>();
            transformComponent->mPosition.z = POISON_TICK_ANIMATION_Z;
            transformComponent->mScale      = POISON_TICK_ANIMATION_SCALE;

            mWorld.AddComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(renderableComponent));
            mWorld.AddComponent<TransformComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(transformComponent));
        }
        else
        {
            encounterStateComponent.mLastMoveSelected                = POISON_TICK_MOVE_NAME;
            encounterStateComponent.mLastMoveCrit                    = false;
            encounterStateComponent.mMoveHadNoEffect                 = false;
            encounterStateComponent.mNothingHappenedFromMoveExecution = false;
            
            // https://bulbapedia.bulbagarden.net/wiki/Poison_(status_condition)#Outside_of_battle
            encounterStateComponent.mOutstandingFloatDamage = ceil(attackingPokemon.mMaxHp / 16.0f);
            encounterStateComponent.mDefenderFloatHealth    = static_cast<float>(attackingPokemon.mHp);

            CompleteAndTransitionTo<HealthDepletionEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PoisonTickAnimationEncounterFlowState::LoadPoisonTickFrames() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    encounterStateComponent.mViewObjects.mBattleAnimationTimer = std::make_unique<Timer>(BATTLE_MOVE_ANIMATION_FRAME_DURATION);

    auto battleAnimationDirPath = ResourceLoadingService::RES_TEXTURES_ROOT +
    (
        encounterStateComponent.mIsOpponentsTurn ?
        ENEMY_POISON_TICK_ANIMATION_DIR :
        PLAYER_POISON_TICK_ANIMATION_DIR
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
    
    SoundService::GetInstance().PlaySfx(POISON_TICK_SFX_NAME);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
