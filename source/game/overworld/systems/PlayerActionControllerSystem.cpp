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
#include "../../common/components/PlayerTagComponent.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../overworld/OverworldConstants.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/components/NpcAiComponent.h"
#include "../../overworld/components/LevelModelComponent.h"
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
    auto& inputStateComponent            = mWorld.GetSingletonComponent<InputStateSingletonComponent>();

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

            if (warpConnectionsComponent.mHasPendingWarpConnection)
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
            
            if (inputStateComponent.mCurrentInputState.at(VirtualActionType::A) == VirtualActionInputState::TAPPED)
            {
                CheckForNpcInteraction(directionComponent.mDirection, movementStateComponent);
                if (GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID)
                {
                    PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                }
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::WEST, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT) == VirtualActionInputState::PRESSED)
            {
                movementStateComponent.mMoving = true;
                ChangePlayerDirectionAndAnimation(Direction::WEST, renderableComponent, directionComponent);
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::EAST, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT) == VirtualActionInputState::PRESSED)
            {
                movementStateComponent.mMoving = true;
                ChangePlayerDirectionAndAnimation(Direction::EAST, renderableComponent, directionComponent);
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::NORTH, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP) == VirtualActionInputState::PRESSED)
            {
                movementStateComponent.mMoving = true;
                ChangePlayerDirectionAndAnimation(Direction::NORTH, renderableComponent, directionComponent);
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::SOUTH, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN) == VirtualActionInputState::PRESSED)
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
            auto& npcTimerComponent      = mWorld.GetComponent<AnimationTimerComponent>(tile.mTileOccupierEntityId);
            auto& npcDirectionComponent  = mWorld.GetComponent<DirectionComponent>(tile.mTileOccupierEntityId);            
            
            const auto newNpcDirection       = GetDirectionFacingDirection(direction);
            npcDirectionComponent.mDirection = newNpcDirection;            
            
            if (mWorld.HasComponent<RenderableComponent>(tile.mTileOccupierEntityId) && npcAiComponent.mMovementType != CharacterMovementType::STATIC)
            {
                auto& npcRenderableComponent = mWorld.GetComponent<RenderableComponent>(tile.mTileOccupierEntityId);
                ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(newNpcDirection), npcRenderableComponent);
            }

            const auto textboxEntityId = CreateTextboxWithDimensions(TextboxType::CHATBOX, 20, 6, 0.0f, -0.6701f, mWorld);
            QueueDialogForTextbox(textboxEntityId, npcAiComponent.mDialog, mWorld);
           
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
