//
//  AnimationSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "AnimationSystem.h"
#include "../components/AnimationTimerComponent.h"
#include "../components/RenderableComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

AnimationSystem::AnimationSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<AnimationTimerComponent, RenderableComponent>();
}

void AnimationSystem::VUpdateAssociatedComponents(const float dt) const
{
    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (ShouldProcessEntity(entityId))
        {
            auto& animationComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);

            animationComponent.mAnimationTimer->Update(dt);

            if (animationComponent.mAnimationTimer->HasTicked())
            {
                animationComponent.mAnimationTimer->Reset();
                
                auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
                const auto& activeAnimationMeshes = renderableComponent.mAnimationsToMeshes.at(renderableComponent.mActiveAnimationNameId);

                renderableComponent.mActiveMeshIndex = (renderableComponent.mActiveMeshIndex + 1) % activeAnimationMeshes.size();
            }                        
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////