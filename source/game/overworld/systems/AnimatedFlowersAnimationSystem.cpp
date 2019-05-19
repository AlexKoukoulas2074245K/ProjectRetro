//
//  AnimatedFlowersAnimationSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 19/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "AnimatedFlowersAnimationSystem.h"
#include "../OverworldConstants.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../components/AnimatedFlowerTagComponent.h"
#include "../components/AnimatedFlowersStateSingletonComponent.h"


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float AnimatedFlowersAnimationSystem::FLOWERS_MOVEMENT_COOLDOWN  = 0.35f;
const int AnimatedFlowersAnimationSystem::FLOWERS_MOVEMENT_COUNTER_MAX = 3;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

AnimatedFlowersAnimationSystem::AnimatedFlowersAnimationSystem(ecs::World& world)
: BaseSystem(world)
{
    InitializeAnimatedFlowersState();
    CalculateAndSetComponentUsageMask<AnimatedFlowerTagComponent, RenderableComponent>();
}

void AnimatedFlowersAnimationSystem::VUpdateAssociatedComponents(const float dt) const
{
    auto& animatedFlowersStateComponent = mWorld.GetSingletonComponent<AnimatedFlowersStateSingletonComponent>();
    animatedFlowersStateComponent.mAnimatedFlowersTimer->Update(dt);
    
    if (animatedFlowersStateComponent.mAnimatedFlowersTimer->HasTicked())
    {
        const auto& activeEntities = mWorld.GetActiveEntities();
        for (const auto& entityId : activeEntities)
        {
            if (ShouldProcessEntity(entityId))
            {
                auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
                
                switch (animatedFlowersStateComponent.mMovementCounter)
                {
                    case 0:
                    case 1: renderableComponent.mActiveAnimationNameId = SOUTH_ANIMATION_NAME_ID; break;
                    case 2: renderableComponent.mActiveAnimationNameId = NORTH_ANIMATION_NAME_ID; break;
                    case 3: renderableComponent.mActiveAnimationNameId = WEST_ANIMATION_NAME_ID; break;
                }
            }
        }
        
        if (++animatedFlowersStateComponent.mMovementCounter > FLOWERS_MOVEMENT_COUNTER_MAX)
        {
            animatedFlowersStateComponent.mMovementCounter = 0;
        }
        
        animatedFlowersStateComponent.mAnimatedFlowersTimer->Reset();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void AnimatedFlowersAnimationSystem::InitializeAnimatedFlowersState() const
{
    auto animatedFlowersStateComponent = std::make_unique<AnimatedFlowersStateSingletonComponent>();
    animatedFlowersStateComponent->mAnimatedFlowersTimer = std::make_unique<Timer>(FLOWERS_MOVEMENT_COOLDOWN);
    
    mWorld.SetSingletonComponent<AnimatedFlowersStateSingletonComponent>(std::move(animatedFlowersStateComponent));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
