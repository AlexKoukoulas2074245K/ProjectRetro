//
//  NpcAiSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 03/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "NpcAiSystem.h"
#include "../components/ActiveLevelSingletonComponent.h"
#include "../components/MovementStateComponent.h"
#include "../components/NpcAiComponent.h"
#include "../utils/LevelUtils.h"
#include "../utils/OverworldCharacterLoadingUtils.h"
#include "../utils/OverworldUtils.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/PokemonUtils.h"
#include "../../common/utils/StringUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/utils/AnimationUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float NpcAiSystem::DYNAMIC_NPC_MIN_MOVEMENT_INITIATION_TIME = 0.5f;
const float NpcAiSystem::DYNAMIC_NPC_MAX_MOVEMENT_INITIATION_TIME = 3.0f;
const float NpcAiSystem::EXCLAMATION_MARK_LIFE_TIME               = 1.0f;

const int NpcAiSystem::TRAINER_LINE_OF_SIGHT      = 3;
const int NpcAiSystem::EXCLAMATION_MARK_ATLAS_COL = 7;
const int NpcAiSystem::EXCLAMATION_MARK_ATLAS_ROW = 46;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

NpcAiSystem::NpcAiSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<NpcAiComponent, DirectionComponent, RenderableComponent>();
}

void NpcAiSystem::VUpdateAssociatedComponents(const float dt) const
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& activeLevelComponent    = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& levelModelComponent     = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
    
    const auto& activeEntities = mWorld.GetActiveEntities();
    
    for (const auto& entityId: activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {
            if (GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID || encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
            {
                auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
                auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);

                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                continue;
            }
            
            auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(entityId);
            
            // Npc engaged in combat
            if (npcAiComponent.mExclamationMarkEntityId != ecs::NULL_ENTITY_ID)
            {
                npcAiComponent.mAiTimer->Update(dt);
                if (npcAiComponent.mAiTimer->HasTicked())
                {
                    npcAiComponent.mAiTimer = nullptr;
                    mWorld.DestroyEntity(npcAiComponent.mExclamationMarkEntityId);
                    npcAiComponent.mExclamationMarkEntityId = ecs::NULL_ENTITY_ID;
                    
                    auto& movementStateComponent = mWorld.GetComponent<MovementStateComponent>(entityId);
                    movementStateComponent.mMoving = true;
                }
            }
            else if
            (
                npcAiComponent.mMovementType == CharacterMovementType::STATIONARY ||
                (npcAiComponent.mIsTrainer && npcAiComponent.mIsDefeated == false && npcAiComponent.mIsEngagedInCombat == false)
            )
            {
                if
                (
                    npcAiComponent.mIsTrainer &&
                    npcAiComponent.mIsGymLeader == false &&
                    npcAiComponent.mIsDefeated == false
                )
                {
                    const auto& movementStateComponent = mWorld.GetComponent<MovementStateComponent>(entityId);
                    
                    if (movementStateComponent.mMoving == false && npcAiComponent.mIsEngagedInCombat == false)
                    {
                        CheckForPlayerDistanceAndEncounterEngagement(entityId);
                    }
                }
                else
                {
                    auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
                    animationTimerComponent.mAnimationTimer->Resume();
                    animationTimerComponent.mAnimationTimer->Update(dt);
                    if (animationTimerComponent.mAnimationTimer->HasTicked())
                    {
                        auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
                        auto& directionComponent  = mWorld.GetComponent<DirectionComponent>(entityId);
                        
                        directionComponent.mDirection = npcAiComponent.mInitDirection;
                        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(npcAiComponent.mInitDirection), renderableComponent);
                    }
                }
            }
            else if (npcAiComponent.mMovementType == CharacterMovementType::DYNAMIC)
            {
                if (npcAiComponent.mIsEngagedInCombat)
                {
                    auto& movementStateComponent   = mWorld.GetComponent<MovementStateComponent>(entityId);
                    auto& playerStateComponent     = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
                    const auto& directionComponent = mWorld.GetComponent<DirectionComponent>(entityId);
                    const auto& targetTile         = GetNeighborTile(movementStateComponent.mCurrentCoords, directionComponent.mDirection, levelModelComponent.mLevelTilemap);
                    
                    if
                    (
                        targetTile.mTileOccupierEntityId != ecs::NULL_ENTITY_ID &&
                        targetTile.mTileOccupierEntityId != entityId &&
                        movementStateComponent.mMoving == false
                    )
                    {
                        auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
                        auto& renderableComponent     = mWorld.GetComponent<RenderableComponent>(entityId);
                        PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                        
                        QueueDialogForChatbox(CreateChatbox(mWorld), npcAiComponent.mDialog, mWorld);
                        
                        playerStateComponent.mLastNpcSpokenToEntityId = entityId;
                    }
                    else
                    {
                        ResumeCurrentlyPlayingAnimation(mWorld.GetComponent<AnimationTimerComponent>(entityId));
                        movementStateComponent.mMoving = true;
                    }
                    
                    return;
                }
                auto& movementStateComponent = mWorld.GetComponent<MovementStateComponent>(entityId);
                
                if (movementStateComponent.mMoving)
                {
                    continue;
                }
                
                // Pause animation timer (pause movement animation)
                auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
                auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
                
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                
                if (npcAiComponent.mAiTimer == nullptr)
                {
                    const auto movementInitiationTime = math::RandomFloat
                    (
                        DYNAMIC_NPC_MIN_MOVEMENT_INITIATION_TIME,
                        DYNAMIC_NPC_MAX_MOVEMENT_INITIATION_TIME
                    );
                    
                    npcAiComponent.mAiTimer = std::make_unique<Timer>(movementInitiationTime);
                }
                else
                {
                    // Update npc timer
                    npcAiComponent.mAiTimer->Update(dt);
                    
                    if (npcAiComponent.mAiTimer->HasTicked())
                    {
                        auto& directionComponent  = mWorld.GetComponent<DirectionComponent>(entityId);
                        
                        const auto newDirection       = static_cast<Direction>(math::RandomInt(0, 3));
                        directionComponent.mDirection = newDirection;
                        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(newDirection), renderableComponent);
                        
                        const auto& targetTile = GetNeighborTile(movementStateComponent.mCurrentCoords, newDirection, levelModelComponent.mLevelTilemap);
                        if (targetTile.mTileTrait == TileTrait::NONE)
                        {
                            ResumeCurrentlyPlayingAnimation(animationTimerComponent);
                            movementStateComponent.mMoving = true;
                        }
                        
                        npcAiComponent.mAiTimer = nullptr;
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void NpcAiSystem::CheckForPlayerDistanceAndEncounterEngagement(const ecs::EntityId npcEntityId) const
{
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    
    const auto& npcMovementStateComponent    = mWorld.GetComponent<MovementStateComponent>(npcEntityId);
    const auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(playerEntityId);
    const auto& npcDirectionComponent        = mWorld.GetComponent<DirectionComponent>(npcEntityId);
    const auto& npcTransformComponent        = mWorld.GetComponent<TransformComponent>(npcEntityId);
    
    auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    //auto& encounterStateComponent            = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    // Check for all tiles in line of sight of npc
    for (int i = 1; i <= TRAINER_LINE_OF_SIGHT; ++i)
    {
        const auto& tileCoords = GetNeighborTileCoords(npcMovementStateComponent.mCurrentCoords, npcDirectionComponent.mDirection, i);
        if (playerMovementStateComponent.mCurrentCoords == tileCoords)
        {
            npcAiComponent.mIsEngagedInCombat = true;
            npcAiComponent.mAiTimer = std::make_unique<Timer>(EXCLAMATION_MARK_LIFE_TIME);
            npcAiComponent.mExclamationMarkEntityId = mWorld.CreateEntity();
            auto exclamationMarkRenderableComponent = CreateRenderableComponentForSprite
            (
                CharacterSpriteData
                (
                    CharacterMovementType::STATIC,
                    EXCLAMATION_MARK_ATLAS_COL,
                    EXCLAMATION_MARK_ATLAS_ROW
                )
            );
            ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::SOUTH), *exclamationMarkRenderableComponent);
            
            auto exclamationMarkTransformComponent       = std::make_unique<TransformComponent>();
            exclamationMarkTransformComponent->mPosition = npcTransformComponent.mPosition;
            exclamationMarkTransformComponent->mRotation = npcTransformComponent.mRotation;
            exclamationMarkTransformComponent->mScale    = npcTransformComponent.mScale;
            
            exclamationMarkTransformComponent->mPosition.y += 1.6f;
            
            mWorld.AddComponent<RenderableComponent>(npcAiComponent.mExclamationMarkEntityId, std::move(exclamationMarkRenderableComponent));
            mWorld.AddComponent<TransformComponent>(npcAiComponent.mExclamationMarkEntityId, std::move(exclamationMarkTransformComponent));
            
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
