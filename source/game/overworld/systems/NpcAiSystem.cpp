//
//  NpcAiSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 03/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "NpcAiSystem.h"
#include "../components/MovementStateComponent.h"
#include "../components/NpcAiComponent.h"
#include "../../common/components/DirectionComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float NpcAiSystem::DYNAMIC_NPC_MIN_MOVEMENT_INITIATION_TIME = 0.5f;
const float NpcAiSystem::DYNAMIC_NPC_MAX_MOVEMENT_INITIATION_TIME = 3.0f;
const float NpcAiSystem::STATIONARY_NPC_RESET_TIME                = 1.5f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

NpcAiSystem::NpcAiSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<NpcAiComponent, DirectionComponent>();
}

void NpcAiSystem::VUpdateAssociatedComponents(const float) const
{
    const auto& activeEntities = mWorld.GetActiveEntities();
    
    for (const auto& entityId: activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(entityId);
            
            if (npcAiComponent.mMovementType == CharacterMovementType::DYNAMIC)
            {
                const auto& movementStateComponent = mWorld.GetComponent<MovementStateComponent>(entityId);
                
                if (movementStateComponent.mMoving)
                {
                    continue;
                }
                
                // Probably reset animation timer here
                
                if (npcAiComponent.mAiTimer == nullptr || npcAiComponent.mAiTimer->HasTicked())
                {
                    const auto
                    npcAiComponent.mAiTimer = std::make_unique<Timer>(
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
