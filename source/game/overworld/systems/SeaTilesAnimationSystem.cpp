//
//  SeaTilesAnimationSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 15/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "SeaTilesAnimationSystem.h"
#include "../components/SeaStateSingletonComponent.h"
#include "../components/SeaTileTagComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../rendering/components/RenderableComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float SeaTilesAnimationSystem::SEA_TILE_MOVEMENT_COOLDOWN = 0.35f;
const float SeaTilesAnimationSystem::SEA_TILE_MOVEMENT_SPEED    = 0.1f;
const int SeaTilesAnimationSystem::SEA_MOVEMENT_COUNTER_MAX     = 2;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

SeaTilesAnimationSystem::SeaTilesAnimationSystem(ecs::World& world)
    : BaseSystem(world)
{    
    InitializeSeaState();
    CalculateAndSetComponentUsageMask<RenderableComponent, TransformComponent, SeaTileTagComponent>();
}

void SeaTilesAnimationSystem::VUpdateAssociatedComponents(const float dt) const
{
    auto& seaStateComponent = mWorld.GetSingletonComponent<SeaStateSingletonComponent>();
    seaStateComponent.mSeaStateTimer->Update(dt);

    if (seaStateComponent.mSeaStateTimer->HasTicked())
    {        
        const auto& activeEntities = mWorld.GetActiveEntities();
        for (const auto& entityId : activeEntities)
        {
            if (ShouldProcessEntity(entityId))
            {
                auto& transformComponent = mWorld.GetComponent<TransformComponent>(entityId);
                transformComponent.mPosition.x += seaStateComponent.mWaveDirectionLeft ? -SEA_TILE_MOVEMENT_SPEED : SEA_TILE_MOVEMENT_SPEED;
            }
        }

        seaStateComponent.mSeaStateTimer->Reset();

        if (seaStateComponent.mWaveDirectionLeft)
        {
            if (--seaStateComponent.mMovementCounter == -SEA_MOVEMENT_COUNTER_MAX)
            {
                seaStateComponent.mWaveDirectionLeft = false;
            }
        }
        else
        {
            if (++seaStateComponent.mMovementCounter == SEA_MOVEMENT_COUNTER_MAX)
            {
                seaStateComponent.mWaveDirectionLeft = true;
            }
        }
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void SeaTilesAnimationSystem::InitializeSeaState() const
{
    auto seaStateComponent = std::make_unique<SeaStateSingletonComponent>();
    seaStateComponent->mSeaStateTimer = std::make_unique<Timer>(SEA_TILE_MOVEMENT_COOLDOWN);

    mWorld.SetSingletonComponent<SeaStateSingletonComponent>(std::move(seaStateComponent));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
