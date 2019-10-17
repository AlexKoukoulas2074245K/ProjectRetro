//
//  BideUnleashAnimationEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 17/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BideUnleashAnimationEncounterFlowState.h"
#include "MoveMissEncounterFlowState.h"
#include "MoveShakeEncounterFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/FileUtils.h"
#include "../utils/OSMessageBox.h"
#include "../utils/PokemonMoveUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 BideUnleashAnimationEncounterFlowState::BIDE_UNLEASH_ANIMATION_SCALE = glm::vec3(2.0f, 2.0f, 1.0f);

const std::string BideUnleashAnimationEncounterFlowState::BATTLE_ANIMATION_MODEL_FILE_NAME  = "battle_anim_quad.obj";
const std::string BideUnleashAnimationEncounterFlowState::PLAYER_BIDE_UNLEASH_ANIMATION_DIR = "battle_animations/BIDE_UNLEASH/";
const std::string BideUnleashAnimationEncounterFlowState::ENEMY_BIDE_UNLEASH_ANIMATION_DIR  = "battle_animations/BIDE_UNLEASH_ENEMY/";
const std::string BideUnleashAnimationEncounterFlowState::BIDE_UNLEASH_SFX_NAME             = "encounter/bide";

const float BideUnleashAnimationEncounterFlowState::BIDE_UNLEASH_ANIMATION_Z = -1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

BideUnleashAnimationEncounterFlowState::BideUnleashAnimationEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    const auto& activePlayerPokemon = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    if (encounterStateComponent.mIsOpponentsTurn && activeOpponentPokemon.mBideAccumulatedDamage == 0)
    {
        const auto mainChatboxEntityId = GetActiveTextboxEntityId(mWorld);
        DeleteTextAtTextboxRow(mainChatboxEntityId, 2, mWorld);
        DeleteTextAtTextboxRow(mainChatboxEntityId, 4, mWorld);
        encounterStateComponent.mLastMoveMiss = true;
        CompleteAndTransitionTo<MoveMissEncounterFlowState>();
    }
    else if (encounterStateComponent.mIsOpponentsTurn == false && activePlayerPokemon.mBideAccumulatedDamage == 0)
    {
        const auto mainChatboxEntityId = GetActiveTextboxEntityId(mWorld);
        DeleteTextAtTextboxRow(mainChatboxEntityId, 2, mWorld);
        DeleteTextAtTextboxRow(mainChatboxEntityId, 4, mWorld);
        encounterStateComponent.mLastMoveMiss = true;
        CompleteAndTransitionTo<MoveMissEncounterFlowState>();
    }
    else
    {
        LoadBideUnleashedAnimationFrames();
    }    
}

void BideUnleashAnimationEncounterFlowState::VUpdate(const float dt)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();   

    const auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    
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
            transformComponent->mPosition.z = BIDE_UNLEASH_ANIMATION_Z;
            transformComponent->mScale      = BIDE_UNLEASH_ANIMATION_SCALE;

            mWorld.AddComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(renderableComponent));
            mWorld.AddComponent<TransformComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(transformComponent));
        }
        else
        {
            encounterStateComponent.mLastMoveSelected                 = BIDE_UNLEASHED_MOVE_NAME;
            encounterStateComponent.mMoveHadNoEffect                  = false;
            encounterStateComponent.mNothingHappenedFromMoveExecution = false;
            encounterStateComponent.mLastMoveCrit                     = false;
            encounterStateComponent.mLastMoveMiss                     = false;

            if (encounterStateComponent.mIsOpponentsTurn)
            { 
                encounterStateComponent.mDefenderFloatHealth    = static_cast<float>(activePlayerPokemon.mHp);
                encounterStateComponent.mOutstandingFloatDamage = static_cast<float>(activeOpponentPokemon.mBideAccumulatedDamage);

                CompleteAndTransitionTo<MoveShakeEncounterFlowState>();                 
            }
            else
            {                                
                encounterStateComponent.mDefenderFloatHealth    = static_cast<float>(activeOpponentPokemon.mHp);
                encounterStateComponent.mOutstandingFloatDamage = static_cast<float>(activePlayerPokemon.mBideAccumulatedDamage);

                CompleteAndTransitionTo<MoveShakeEncounterFlowState>();                
            }                                                            
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void BideUnleashAnimationEncounterFlowState::LoadBideUnleashedAnimationFrames() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& resourceLoadingService  = ResourceLoadingService::GetInstance();

    encounterStateComponent.mViewObjects.mBattleAnimationTimer = std::make_unique<Timer>(BATTLE_MOVE_ANIMATION_FRAME_DURATION);

    auto battleAnimationDirPath = ResourceLoadingService::RES_TEXTURES_ROOT +
    (
        encounterStateComponent.mIsOpponentsTurn ?
        ENEMY_BIDE_UNLEASH_ANIMATION_DIR :
        PLAYER_BIDE_UNLEASH_ANIMATION_DIR
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
    
    SoundService::GetInstance().PlaySfx(BIDE_UNLEASH_SFX_NAME);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
