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
#include "../../common/components/PlayerTagComponent.h"
#include "../../common/components/DirectionComponent.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../overworld/OverworldConstants.h"
#include "../../overworld/components/MovementStateComponent.h"
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
            auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
            auto& movementStateComponent  = mWorld.GetComponent<MovementStateComponent>(entityId);
            auto& renderableComponent     = mWorld.GetComponent<RenderableComponent>(entityId);
            
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
            
            if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT) == VirtualActionInputState::TAPPED)
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
