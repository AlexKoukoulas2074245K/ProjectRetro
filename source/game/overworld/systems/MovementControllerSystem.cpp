//
//  MovementControllerSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 03/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MovementControllerSystem.h"
#include "../components/ActiveLevelSingletonComponent.h"
#include "../components/JumpingStateComponent.h"
#include "../components/LevelModelComponent.h"
#include "../components/MovementStateComponent.h"
#include "../components/WarpConnectionsSingletonComponent.h"
#include "../utils/EncounterUtils.h"
#include "../utils/LevelUtils.h"
#include "../utils/LevelLoadingUtils.h"
#include "../utils/MovementUtils.h"
#include "../utils/OverworldUtils.h"
#include "../utils/WarpConnectionsUtils.h"
#include "../../common/GameConstants.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/components/PlayerTagComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../common/utils/PokemonUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../resources/ResourceLoadingService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float MovementControllerSystem::CHARACTER_MOVEMENT_SPEED = 4 * GAME_TILE_SIZE;
const std::string MovementControllerSystem::JUMP_SHADOW_SPRITE_NAME = "jump_shadow";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MovementControllerSystem::MovementControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<MovementStateComponent, DirectionComponent, TransformComponent>();
}

void MovementControllerSystem::VUpdateAssociatedComponents(const float dt) const
{
    auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    auto& levelModelComponent  = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));

    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto hasPlayerTag        = mWorld.HasComponent<PlayerTagComponent>(entityId);
            const auto& directionComponent = mWorld.GetComponent<DirectionComponent>(entityId);
            auto& transformComponent       = mWorld.GetComponent<TransformComponent>(entityId);
            auto& movementStateComponent   = mWorld.GetComponent<MovementStateComponent>(entityId);
                        
            if (movementStateComponent.mMoving == false)
            {
                continue;
            }            

            const auto& currentTileCoords = movementStateComponent.mCurrentCoords;            
            const auto targetTileCoords = GetNeighborTileCoords(currentTileCoords, directionComponent.mDirection);
            auto& currentTile = levelModelComponent.mLevelTilemap.at(currentTileCoords.mRow).at(currentTileCoords.mCol);
            
            // Bounds check
            if 
            (   
                targetTileCoords.mCol < 0 ||
                targetTileCoords.mRow < 0 ||
                targetTileCoords.mCol >= levelModelComponent.mCols ||
                targetTileCoords.mRow >= levelModelComponent.mRows
            )
            {
                movementStateComponent.mMoving = false;
                continue;
            }

            // Interaction Warp (Doors where a direction has to be pressed to warp) check
            if (currentTile.mTileTrait == TileTrait::PRESS_WARP)
            {
                const auto directionNeededForWarp = CalculateMovementDirectionToActivateInteractionWarp
                (
                    currentTileCoords, 
                    levelModelComponent.mLevelTilemap
                );

                if (directionComponent.mDirection == directionNeededForWarp)
                {                       
                    movementStateComponent.mMoving = false;
                    
                    auto& warpConnectionsComponent = mWorld.GetSingletonComponent<WarpConnectionsSingletonComponent>();
                    warpConnectionsComponent.mHasPendingWarpConnection = true;
                    warpConnectionsComponent.mShouldPlayTransitionAnimation = true;
                    continue;
                }
            }

            // Safe to now get the actual target tile            
            auto& targetTile  = GetTile(targetTileCoords, levelModelComponent.mLevelTilemap);

            // Solidity check
            if (targetTile.mTileTrait == TileTrait::SOLID)
            {
                movementStateComponent.mMoving = false;
                continue;
            }                        
            
            // Occupier checks
            if (targetTile.mTileOccupierType == TileOccupierType::NPC && targetTile.mTileOccupierEntityId != entityId)
            {
                movementStateComponent.mMoving = false;
                continue;
            }

            if (targetTile.mTileOccupierType == TileOccupierType::PLAYER && targetTile.mTileOccupierEntityId != entityId)
            {
                movementStateComponent.mMoving = false;
                continue;
            }

            // Jumping ledge check
            if (targetTile.mTileTrait == TileTrait::JUMPING_LEDGE_BOT)
            {
                if ((directionComponent.mDirection != Direction::SOUTH) || !hasPlayerTag)
                {
                    movementStateComponent.mMoving = false;
                    continue;
                }
                else if (mWorld.HasComponent<JumpingStateComponent>(entityId) == false)
                {
                    StartJump(entityId);
                }
            } 
            else if (targetTile.mTileTrait == TileTrait::JUMPING_LEDGE_LEFT)
            {
                if ((directionComponent.mDirection != Direction::WEST) || !hasPlayerTag)
                {
                    movementStateComponent.mMoving = false;
                    continue;
                }
                else if (mWorld.HasComponent<JumpingStateComponent>(entityId) == false)
                {
                    StartJump(entityId);
                }
            }
            else if (targetTile.mTileTrait == TileTrait::JUMPING_LEDGE_RIGHT)
            {
                if ((directionComponent.mDirection != Direction::EAST) || !hasPlayerTag)
                {
                    movementStateComponent.mMoving = false;
                    continue;
                }
                else if (mWorld.HasComponent<JumpingStateComponent>(entityId) == false)
                {
                    StartJump(entityId);
                }
            }

            // Clear occupier status of the current tile
            currentTile.mTileOccupierEntityId = ecs::NULL_ENTITY_ID;
            currentTile.mTileOccupierType     = TileOccupierType::NONE;

            // Set occupier status on the target tile
            targetTile.mTileOccupierEntityId = entityId;
            targetTile.mTileOccupierType     = hasPlayerTag ? TileOccupierType::PLAYER : TileOccupierType::NPC;

            // Move the transform to target by a tick
            const auto moveOutcome = MoveToTargetPosition
            (
                TileCoordsToPosition(targetTileCoords),
                CHARACTER_MOVEMENT_SPEED, 
                dt, 
                transformComponent.mPosition
            );

            // Update jumping displacement 
            if (hasPlayerTag && mWorld.HasComponent<JumpingStateComponent>(entityId))
            {
                auto& jumpingStateComponent = mWorld.GetComponent<JumpingStateComponent>(entityId);
                jumpingStateComponent.mJumpingTimer->Update(dt);

                // Simulate jump
                SimulateJumpDisplacement(dt, jumpingStateComponent, transformComponent);                
            }

            // Target position reached in this frame
            if (moveOutcome == MoveOutcome::COMPLETED)
            {
                const auto newPosition = TileCoordsToPosition(targetTileCoords);
                
                transformComponent.mPosition.x = newPosition.x;                
                transformComponent.mPosition.z = newPosition.z;

                movementStateComponent.mMoving        = false;
                movementStateComponent.mCurrentCoords = targetTileCoords;

                // Warp tile flow
                if 
                (
                    (targetTile.mTileTrait == TileTrait::WARP || targetTile.mTileTrait == TileTrait::NO_ANIM_WARP) && 
                    hasPlayerTag
                )
                {
                    auto& warpConnectionsComponent = mWorld.GetSingletonComponent<WarpConnectionsSingletonComponent>();
                    warpConnectionsComponent.mHasPendingWarpConnection = true;
                    warpConnectionsComponent.mShouldPlayTransitionAnimation = targetTile.mTileTrait == TileTrait::WARP;
                }

                // Encounter tile flow
                if 
                (
                    targetTile.mTileTrait == TileTrait::ENCOUNTER &&
                    DoesLevelHaveWildEncounters(levelModelComponent) &&
                    WildEncounterRNGTriggered(levelModelComponent) &&
                    hasPlayerTag
                )
                {
                    // Delete jump if landing coincided with encounter
                    if (mWorld.HasComponent<JumpingStateComponent>(entityId))
                    {
                        DestroyJumpSprite(entityId);
                    }

                    const auto& encounterInfo = SelectRandomWildEncounter(levelModelComponent);
                    
                    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
                    
                    encounterStateComponent.mActiveEncounterType = EncounterType::TRAINER ;
                    encounterStateComponent.mOpponentTrainerSpeciesName = StringId("RIVAL_1");
                    encounterStateComponent.mOpponentTrainerName = StringId("Gary");
                    encounterStateComponent.mOpponentTrainerDefeatedText = StringId("Gary: WHAT?#Unbelievable!#I picked the#wrong POK^MON!");
                    encounterStateComponent.mOpponentPokemonRoster.push_back
                    (
                        CreatePokemon
                        (
                            StringId("EEVEE"),
                            true,
                            encounterInfo.mPokemonLevel,
                            mWorld
                        )
                    );
                    

                    encounterStateComponent.mActivePlayerPokemonRosterIndex   = 0;
                    encounterStateComponent.mActiveOpponentPokemonRosterIndex = 0;
                    
                    return;
                }

                // Jumping ledge tile flow
                if 
                (
                    targetTile.mTileTrait == TileTrait::JUMPING_LEDGE_BOT ||
                    targetTile.mTileTrait == TileTrait::JUMPING_LEDGE_LEFT ||
                    targetTile.mTileTrait == TileTrait::JUMPING_LEDGE_RIGHT
                )
                {
                    // Midway during a ledge jump, continue moving to the next tile
                    movementStateComponent.mMoving = true;
                }
                else
                {
                    // After ledge jump correct any floating precision errors on y and reset to ground level
                    transformComponent.mPosition.y = newPosition.y;

                    // Delete component at the end of the jump
                    if (mWorld.HasComponent<JumpingStateComponent>(entityId))
                    {
                        DestroyJumpSprite(entityId);
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void MovementControllerSystem::StartJump(const ecs::EntityId playerEntityId) const
{
    auto jumpingStateComponent = std::make_unique<JumpingStateComponent>();

    jumpingStateComponent->mJumpingTimer  = std::make_unique<Timer>(JUMP_DURATION_IN_SECONDS);
    jumpingStateComponent->mJumpYVelocity = JUMP_INIT_VELOCITY;    

    // Create jump shadow sprite
    const auto jumpShadowEntityId = mWorld.CreateEntity();
    const auto& playerTransform = mWorld.GetComponent<TransformComponent>(playerEntityId);

    auto jumpShadowTransformComponent = std::make_unique<TransformComponent>();
    
    jumpShadowTransformComponent->mPosition.x += playerTransform.mPosition.x;
    jumpShadowTransformComponent->mPosition.y -= GAME_TILE_SIZE / 2.0f - 0.01f;
    jumpShadowTransformComponent->mPosition.z += playerTransform.mPosition.z;

    auto jumpShadowRenderableComponent = std::make_unique<RenderableComponent>();
    jumpShadowRenderableComponent->mShaderNameId = StringId("basic");
    jumpShadowRenderableComponent->mAnimationsToMeshes[StringId("default")].
        push_back(ResourceLoadingService::GetInstance().
            LoadResource(ResourceLoadingService::RES_MODELS_ROOT +
                JUMP_SHADOW_SPRITE_NAME + ".obj"));
    jumpShadowRenderableComponent->mActiveAnimationNameId = StringId("default");
    jumpShadowRenderableComponent->mTextureResourceId =
        ResourceLoadingService::GetInstance().LoadResource(
            ResourceLoadingService::RES_TEXTURES_ROOT +
            JUMP_SHADOW_SPRITE_NAME + ".png");
    jumpShadowRenderableComponent->mRenderableLayer = RenderableLayer::LEVEL_FLOOR_LEVEL;

    mWorld.AddComponent<TransformComponent>(jumpShadowEntityId, std::move(jumpShadowTransformComponent));
    mWorld.AddComponent<RenderableComponent>(jumpShadowEntityId, std::move(jumpShadowRenderableComponent));

    jumpingStateComponent->mJumpShadowSpriteEntityid = jumpShadowEntityId;

    mWorld.AddComponent<JumpingStateComponent>(playerEntityId, std::move(jumpingStateComponent));
}

void MovementControllerSystem::SimulateJumpDisplacement(const float dt, JumpingStateComponent& jumpStateComponent, TransformComponent& transformComponent) const
{
    const auto previousJumpDisplacement = jumpStateComponent.mJumpYDisplacement;

    jumpStateComponent.mJumpYVelocity     += JUMP_GRAVITY * dt;
    jumpStateComponent.mJumpYDisplacement += jumpStateComponent.mJumpYVelocity * dt;

    // Add delta jump displacement to y position
    transformComponent.mPosition.y += jumpStateComponent.mJumpYDisplacement - previousJumpDisplacement;

    // Update position of jump shadow sprite
    auto& jumpShadowTransform = mWorld.GetComponent<TransformComponent>(jumpStateComponent.mJumpShadowSpriteEntityid);
    jumpShadowTransform.mPosition.x = transformComponent.mPosition.x;
    jumpShadowTransform.mPosition.z = transformComponent.mPosition.z;    
}

void MovementControllerSystem::DestroyJumpSprite(const ecs::EntityId entityId) const
{
    auto& jumpingStateComponent = mWorld.GetComponent<JumpingStateComponent>(entityId);
    mWorld.DestroyEntity(jumpingStateComponent.mJumpShadowSpriteEntityid);
    mWorld.RemoveComponent<JumpingStateComponent>(entityId);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
