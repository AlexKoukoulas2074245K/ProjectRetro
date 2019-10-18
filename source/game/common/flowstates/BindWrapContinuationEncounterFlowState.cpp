//
//  BindWrapContinuationEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BindWrapContinuationEncounterFlowState.h"
#include "MoveShakeEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/FileUtils.h"
#include "../utils/PokemonMoveUtils.h"
#include "../utils/OSMessageBox.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 BindWrapContinuationEncounterFlowState::BIND_WRAP_ANIMATION_SCALE = glm::vec3(2.0f, 2.0f, 1.0f);

const std::string BindWrapContinuationEncounterFlowState::BATTLE_ANIMATION_MODEL_FILE_NAME = "battle_anim_quad.obj";
const std::string BindWrapContinuationEncounterFlowState::BIND_WRAP_ANIMATION_DIR          = "battle_animations/BIND/";
const std::string BindWrapContinuationEncounterFlowState::BIND_WRAP_SFX_NAME               = "encounter/bind";

const float BindWrapContinuationEncounterFlowState::BIND_WRAP_ANIMATION_Z = -1.0f;
const float BindWrapContinuationEncounterFlowState::BATTLE_MOVE_ANIMATION_ENEMY_Y_DISPLACEMENT = 0.55f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

BindWrapContinuationEncounterFlowState::BindWrapContinuationEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
    , mHasDisplayedText(false)
{    
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    const auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    LoadBindWrapFrames();

    std::string continuationAttackText = "";

    if (activeOpponentPokemon.mBindingOrWrappingOpponentCounter > -1)
    {
        continuationAttackText += "Enemy " + activeOpponentPokemon.mName.GetString();
    }
    else
    {
        continuationAttackText += activePlayerPokemon.mName.GetString();
    }

    continuationAttackText += "'s#attack continues!+FREEZE";

    QueueDialogForChatbox(CreateChatbox(mWorld), continuationAttackText, mWorld);
}

void BindWrapContinuationEncounterFlowState::VUpdate(const float dt)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();    
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    const auto& activePlayerPokemon = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    if (!mHasDisplayedText)
    {
        if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
        {
            mHasDisplayedText = true;
            SoundService::GetInstance().PlaySfx(BIND_WRAP_SFX_NAME);
        }
    }
    else
    {        
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

                auto transformComponent = std::make_unique<TransformComponent>();
                transformComponent->mPosition.z = BIND_WRAP_ANIMATION_Z;
                transformComponent->mScale = BIND_WRAP_ANIMATION_SCALE;                

                if (activeOpponentPokemon.mBindingOrWrappingOpponentCounter > -1)
                {
                    // Flip move animation horizontally
                    transformComponent->mScale.x = -transformComponent->mScale.x;
                    transformComponent->mPosition.y = -BATTLE_MOVE_ANIMATION_ENEMY_Y_DISPLACEMENT;
                }

                mWorld.AddComponent<RenderableComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(renderableComponent));
                mWorld.AddComponent<TransformComponent>(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId, std::move(transformComponent));
            }
            else
            {
                encounterStateComponent.mLastMoveSelected = StringId("BIND");                
                encounterStateComponent.mMoveHadNoEffect = false;
                encounterStateComponent.mNothingHappenedFromMoveExecution = false;

                if (activeOpponentPokemon.mBindingOrWrappingOpponentCounter > -1)
                {
                    encounterStateComponent.mIsOpponentsTurn        = true;
                    encounterStateComponent.mOutstandingFloatDamage = static_cast<float>(activeOpponentPokemon.mBindingOrWrappingContinuationDamage);
                    encounterStateComponent.mDefenderFloatHealth    = static_cast<float>(activePlayerPokemon.mHp);
                }
                else
                {
                    encounterStateComponent.mIsOpponentsTurn        = false;
                    encounterStateComponent.mOutstandingFloatDamage = static_cast<float>(activePlayerPokemon.mBindingOrWrappingContinuationDamage);
                    encounterStateComponent.mDefenderFloatHealth    = static_cast<float>(activeOpponentPokemon.mHp);
                }                
                
                CompleteAndTransitionTo<MoveShakeEncounterFlowState>();
            }
        }
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void BindWrapContinuationEncounterFlowState::LoadBindWrapFrames() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    encounterStateComponent.mViewObjects.mBattleAnimationTimer = std::make_unique<Timer>(BATTLE_MOVE_ANIMATION_FRAME_DURATION);

    auto battleAnimationDirPath = ResourceLoadingService::RES_TEXTURES_ROOT + BIND_WRAP_ANIMATION_DIR;     

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
