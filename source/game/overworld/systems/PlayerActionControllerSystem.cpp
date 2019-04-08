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
#include "../../common/components/PlayerTagComponent.h"
#include "../../common/components/DirectionComponent.h"
#include "../../input/components/InputStateComponent.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../overworld/components/MovementStateComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const StringId PlayerActionControllerSystem::NORTH_ANIMATION_NAME_ID = StringId("north");
const StringId PlayerActionControllerSystem::SOUTH_ANIMATION_NAME_ID = StringId("south");
const StringId PlayerActionControllerSystem::WEST_ANIMATION_NAME_ID  = StringId("west");
const StringId PlayerActionControllerSystem::EAST_ANIMATION_NAME_ID  = StringId("east");

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerActionControllerSystem::PlayerActionControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<AnimationTimerComponent, DirectionComponent, PlayerTagComponent, RenderableComponent, MovementStateComponent>();
}

void PlayerActionControllerSystem::VUpdate(const float) const
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateComponent>();
    
    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
            auto& movementStateComponent        = mWorld.GetComponent<MovementStateComponent>(entityId);
            auto& renderableComponent           = mWorld.GetComponent<RenderableComponent>(entityId);
            auto& directionComponent            = mWorld.GetComponent<DirectionComponent>(entityId);
            
            if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT) == VirtualActionInputState::TAPPED)
            {
                directionComponent.mDirection              = Direction::WEST;
                renderableComponent.mActiveAnimationNameId = WEST_ANIMATION_NAME_ID;
                renderableComponent.mActiveMeshIndex       = 0;
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT) == VirtualActionInputState::PRESSED)
            {
                directionComponent.mDirection              = Direction::WEST;
                renderableComponent.mActiveAnimationNameId = WEST_ANIMATION_NAME_ID;
                movementStateComponent.mMoving             = true;
                animationTimerComponent.mAnimationTimer->Resume();
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT) == VirtualActionInputState::TAPPED)
            {
                directionComponent.mDirection              = Direction::EAST;
                renderableComponent.mActiveAnimationNameId = EAST_ANIMATION_NAME_ID;
                renderableComponent.mActiveMeshIndex       = 0;
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT) == VirtualActionInputState::PRESSED)
            {
                directionComponent.mDirection              = Direction::EAST;
                renderableComponent.mActiveAnimationNameId = EAST_ANIMATION_NAME_ID;
                movementStateComponent.mMoving             = true;
                animationTimerComponent.mAnimationTimer->Resume();
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP) == VirtualActionInputState::TAPPED)
            {
                directionComponent.mDirection              = Direction::NORTH;
                renderableComponent.mActiveAnimationNameId = NORTH_ANIMATION_NAME_ID;
                renderableComponent.mActiveMeshIndex       = 0;
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP) == VirtualActionInputState::PRESSED)
            {
                directionComponent.mDirection              = Direction::NORTH;
                renderableComponent.mActiveAnimationNameId = NORTH_ANIMATION_NAME_ID;
                movementStateComponent.mMoving             = true;
                animationTimerComponent.mAnimationTimer->Resume();
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN) == VirtualActionInputState::TAPPED)
            {
                directionComponent.mDirection              = Direction::SOUTH;
                renderableComponent.mActiveAnimationNameId = SOUTH_ANIMATION_NAME_ID;
                renderableComponent.mActiveMeshIndex       = 0;
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN) == VirtualActionInputState::PRESSED)
            {
                directionComponent.mDirection              = Direction::SOUTH;
                renderableComponent.mActiveAnimationNameId = SOUTH_ANIMATION_NAME_ID;
                movementStateComponent.mMoving             = true;
                animationTimerComponent.mAnimationTimer->Resume();
            }
            // All movement keys released and is currently not moving to another tile
            else if (movementStateComponent.mMoving == false)
            {
                renderableComponent.mActiveMeshIndex = 0;
                animationTimerComponent.mAnimationTimer->Pause();
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
