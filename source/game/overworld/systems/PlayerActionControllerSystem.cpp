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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerActionControllerSystem::PlayerActionControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<PlayerTagComponent, DirectionComponent>();
}

void PlayerActionControllerSystem::VUpdate(const float) const
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateComponent>();
    
    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (ShouldProcessEntity(entityId))
        {
            auto& directionComponent = mWorld.GetComponent<DirectionComponent>(entityId);
            
            if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT) == VirtualActionInputState::TAPPED)
            {
                directionComponent.mDirection = Direction::WEST;
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT) == VirtualActionInputState::PRESSED)
            {
                directionComponent.mDirection = Direction::EAST;
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP) == VirtualActionInputState::PRESSED)
            {
                directionComponent.mDirection = Direction::NORTH;
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN) == VirtualActionInputState::PRESSED)
            {
                directionComponent.mDirection = Direction::SOUTH;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
