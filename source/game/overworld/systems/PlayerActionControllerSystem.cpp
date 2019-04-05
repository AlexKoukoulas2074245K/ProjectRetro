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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerActionControllerSystem::PlayerActionControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<AnimationTimerComponent, DirectionComponent, PlayerTagComponent, RenderableComponent>();
}

void PlayerActionControllerSystem::VUpdate(const float) const
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateComponent>();
    
    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
            auto& renderableComponent           = mWorld.GetComponent<RenderableComponent>(entityId);
            auto& directionComponent            = mWorld.GetComponent<DirectionComponent>(entityId);
            
            if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT) == VirtualActionInputState::TAPPED)
            {
                directionComponent.mDirection              = Direction::WEST;
                renderableComponent.mActiveAnimationNameId = StringId("west");
                renderableComponent.mActiveMeshIndex       = 0;
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT) == VirtualActionInputState::PRESSED)
            {
                directionComponent.mDirection              = Direction::WEST;
                renderableComponent.mActiveAnimationNameId = StringId("west");
                animationTimerComponent.mAnimationTimer->Resume();
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT) == VirtualActionInputState::TAPPED)
            {
                directionComponent.mDirection              = Direction::EAST;
                renderableComponent.mActiveAnimationNameId = StringId("east");
                renderableComponent.mActiveMeshIndex       = 0;
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT) == VirtualActionInputState::PRESSED)
            {
                directionComponent.mDirection              = Direction::EAST;
                renderableComponent.mActiveAnimationNameId = StringId("east");
                animationTimerComponent.mAnimationTimer->Resume();
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP) == VirtualActionInputState::TAPPED)
            {
                directionComponent.mDirection              = Direction::NORTH;
                renderableComponent.mActiveAnimationNameId = StringId("north");
                renderableComponent.mActiveMeshIndex       = 0;
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP) == VirtualActionInputState::PRESSED)
            {
                directionComponent.mDirection              = Direction::NORTH;
                renderableComponent.mActiveAnimationNameId = StringId("north");
                animationTimerComponent.mAnimationTimer->Resume();
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN) == VirtualActionInputState::TAPPED)
            {
                directionComponent.mDirection              = Direction::SOUTH;
                renderableComponent.mActiveAnimationNameId = StringId("south");
                renderableComponent.mActiveMeshIndex       = 0;
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN) == VirtualActionInputState::PRESSED)
            {
                directionComponent.mDirection              = Direction::SOUTH;
                renderableComponent.mActiveAnimationNameId = StringId("south");
                animationTimerComponent.mAnimationTimer->Resume();
            }
            // All movement keys released
            else            
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
