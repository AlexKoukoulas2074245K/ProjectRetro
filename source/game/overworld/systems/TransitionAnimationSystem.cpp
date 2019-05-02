//
//  TransitionAnimationSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TransitionAnimationSystem.h"
#include "../components/TransitionAnimationStateSingletonComponent.h"
#include "../components/WarpConnectionsSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float TransitionAnimationSystem::TRANSITION_STEP_DURATION = 0.13f;
const int TransitionAnimationSystem::TRANSITION_STEPS_COUNT = 3;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TransitionAnimationSystem::TransitionAnimationSystem(ecs::World& world)
    : BaseSystem(world)
{
    mWorld.SetSingletonComponent<TransitionAnimationStateSingletonComponent>(std::make_unique<TransitionAnimationStateSingletonComponent>());
}

void TransitionAnimationSystem::VUpdateAssociatedComponents(const float dt) const
{    
    const auto& warpConnectionsComponent    = mWorld.GetSingletonComponent<WarpConnectionsSingletonComponent>();
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();

    if (transitionAnimationStateComponent.mIsPlayingTransitionAnimation)
    {
        transitionAnimationStateComponent.mAnimationTimer->Update(dt);
        if (transitionAnimationStateComponent.mAnimationTimer->HasTicked())
        {
            transitionAnimationStateComponent.mAnimationTimer->Reset();
            if (++transitionAnimationStateComponent.mAnimationProgressionStep > TRANSITION_STEPS_COUNT)
            {
    
                transitionAnimationStateComponent.mAnimationProgressionStep = 0;
                transitionAnimationStateComponent.mIsPlayingTransitionAnimation = false;
            }            
        }
    }
    else if (warpConnectionsComponent.mHasPendingWarpConnection)
    {
        transitionAnimationStateComponent.mIsPlayingTransitionAnimation = true;
        transitionAnimationStateComponent.mAnimationProgressionStep     = 0;
        transitionAnimationStateComponent.mAnimationTimer               = std::make_unique<Timer>(TRANSITION_STEP_DURATION);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
