//
//  PlayerActionControllerSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PlayerActionControllerSystem.h"
#include "MovementControllerSystem.h"
#include "../components/WarpConnectionsSingletonComponent.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/components/PlayerTagComponent.h"
#include "../../common/flowstates/MainMenuOverworldFlowState.h"
#include "../../common/utils/PokemonItemsUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../overworld/OverworldConstants.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/LevelModelComponent.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/components/NpcAiComponent.h"
#include "../../overworld/components/TransitionAnimationStateSingletonComponent.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../overworld/utils/OverworldUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerActionControllerSystem::PlayerActionControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask
    <
        AnimationTimerComponent, 
        DirectionComponent, 
        PlayerTagComponent, 
        RenderableComponent, 
        MovementStateComponent
    >();
}

void PlayerActionControllerSystem::VUpdateAssociatedComponents(const float) const
{    
    const auto& warpConnectionsComponent = mWorld.GetSingletonComponent<WarpConnectionsSingletonComponent>();
    const auto& encounterStateComponent  = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& transitionStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    auto& inputStateComponent            = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& playerStateComponent           = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    if (playerStateComponent.mPendingItemToBeAdded != StringId())
    {
        AddItemToBag(playerStateComponent.mPendingItemToBeAdded, mWorld);
        playerStateComponent.mPendingItemToBeAdded = StringId();
        
        // Update npc dialog
        auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(playerStateComponent.mLastNpcSpokenToEntityId);
        if (npcAiComponent.mSideDialogs.size() > 0)
        {
            npcAiComponent.mDialog = npcAiComponent.mSideDialogs[0];
        }
    }
    
    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (ShouldProcessEntity(entityId))
        {
            auto& animationTimerComponent  = mWorld.GetComponent<AnimationTimerComponent>(entityId);
            auto& movementStateComponent   = mWorld.GetComponent<MovementStateComponent>(entityId);
            auto& renderableComponent      = mWorld.GetComponent<RenderableComponent>(entityId);
            
            if (movementStateComponent.mMoving)
            {
                continue;
            }
            
            if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
            {
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                continue;
            }
            
            if (transitionStateComponent.mIsPlayingTransitionAnimation)
            {
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                continue;
            }
            
            if (warpConnectionsComponent.mHasPendingWarpConnection)
            {
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                continue;
            }


            if (GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID)
            {
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                continue;
            }

            if (IsAnyOverworldFlowCurrentlyRunning(mWorld))
            {
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                continue;
            }

            if (inputStateComponent.mHasBeenConsumed)
            {
                continue;
            }
                        
            // We'll set the movement direction and intent of movement (mMoving) 
            // in this system's udpate, and do the checks (and possibly revert the intent) in MovementControllerSystem
            auto& directionComponent = mWorld.GetComponent<DirectionComponent>(entityId);
            
            if (inputStateComponent.mCurrentInputState.at(VirtualActionType::START_BUTTON) == VirtualActionInputState::TAPPED)
            {
                StartOverworldFlowState<MainMenuOverworldFlowState>(mWorld);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::A_BUTTON) == VirtualActionInputState::TAPPED)
            {
                CheckForNpcInteraction(directionComponent.mDirection, movementStateComponent);
                if (GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID)
                {
                    PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                }
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT_ARROW) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::WEST, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT_ARROW) == VirtualActionInputState::PRESSED)
            {
                movementStateComponent.mMoving = true;
                ChangePlayerDirectionAndAnimation(Direction::WEST, renderableComponent, directionComponent);
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT_ARROW) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::EAST, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT_ARROW) == VirtualActionInputState::PRESSED)
            {
                movementStateComponent.mMoving = true;
                ChangePlayerDirectionAndAnimation(Direction::EAST, renderableComponent, directionComponent);
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP_ARROW) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::NORTH, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP_ARROW) == VirtualActionInputState::PRESSED)
            {
                movementStateComponent.mMoving = true;
                ChangePlayerDirectionAndAnimation(Direction::NORTH, renderableComponent, directionComponent);
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN_ARROW) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::SOUTH, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN_ARROW) == VirtualActionInputState::PRESSED)
            {
                movementStateComponent.mMoving = true;
                ChangePlayerDirectionAndAnimation(Direction::SOUTH, renderableComponent, directionComponent);
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            }
            // All movement keys released and is currently not moving to another tile
            else
            {
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
            }

            inputStateComponent.mHasBeenConsumed = true;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PlayerActionControllerSystem::CheckForNpcInteraction
(
    const Direction direction,
    const MovementStateComponent& movementStateComponent
) const
{
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& levelModelComponent  = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
    const auto& tile = GetNeighborTile(movementStateComponent.mCurrentCoords, direction, levelModelComponent.mLevelTilemap);
    
    if (tile.mTileOccupierType == TileOccupierType::NPC)
    {
        const auto& npcMovementState = mWorld.GetComponent<MovementStateComponent>(tile.mTileOccupierEntityId);
        
        // Disallow talking to moving npc
        if (npcMovementState.mMoving == false)
        {
            const auto& npcAiComponent   = mWorld.GetComponent<NpcAiComponent>(tile.mTileOccupierEntityId);
            auto& playerStateComponent   = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
            auto& npcTimerComponent      = mWorld.GetComponent<AnimationTimerComponent>(tile.mTileOccupierEntityId);
            auto& npcDirectionComponent  = mWorld.GetComponent<DirectionComponent>(tile.mTileOccupierEntityId);            
            
            const auto newNpcDirection       = GetDirectionFacingDirection(direction);
            npcDirectionComponent.mDirection = newNpcDirection;            
            
            if (mWorld.HasComponent<RenderableComponent>(tile.mTileOccupierEntityId) && npcAiComponent.mMovementType != CharacterMovementType::STATIC)
            {
                auto& npcRenderableComponent = mWorld.GetComponent<RenderableComponent>(tile.mTileOccupierEntityId);
                ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(newNpcDirection), npcRenderableComponent);
            }
            
            QueueDialogForChatbox(CreateChatbox(mWorld), npcAiComponent.mDialog, mWorld);
           
            playerStateComponent.mLastNpcSpokenToEntityId = tile.mTileOccupierEntityId;
            npcTimerComponent.mAnimationTimer->Reset();
        }
    }
}

void PlayerActionControllerSystem::ChangePlayerDirectionAndAnimation
(
    const Direction direction,
    RenderableComponent& playerRenderableComponent,
    DirectionComponent& playerDirectionComponent
) const
{
    playerDirectionComponent.mDirection = direction;
    ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(direction), playerRenderableComponent);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
