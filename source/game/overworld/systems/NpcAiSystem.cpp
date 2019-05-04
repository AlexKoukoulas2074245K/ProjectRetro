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
#include "../components/ActiveLevelSingletonComponent.h"
#include "../components/MovementStateComponent.h"
#include "../components/NpcAiComponent.h"
#include "../utils/LevelUtils.h"
#include "../utils/OverworldUtils.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/StringUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/utils/AnimationUtils.h"

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
    CalculateAndSetComponentUsageMask<NpcAiComponent, DirectionComponent, RenderableComponent>();
}

void NpcAiSystem::VUpdateAssociatedComponents(const float dt) const
{
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& levelModelComponent  = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
    
    const auto& activeEntities      = mWorld.GetActiveEntities();
    
    for (const auto& entityId: activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {
            auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(entityId);
            
            if (npcAiComponent.mMovementType == CharacterMovementType::DYNAMIC)
            {
                auto& movementStateComponent = mWorld.GetComponent<MovementStateComponent>(entityId);
                
                if (movementStateComponent.mMoving)
                {
                    continue;
                }
                
                // Pause animation timer (pause movement animation)
                auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
                auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
                
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                
                if (npcAiComponent.mAiTimer == nullptr)
                {
                    const auto movementInitiationTime = math::RandomFloat
                    (
                        DYNAMIC_NPC_MIN_MOVEMENT_INITIATION_TIME,
                        DYNAMIC_NPC_MAX_MOVEMENT_INITIATION_TIME
                    );
                    
                    npcAiComponent.mAiTimer = std::make_unique<Timer>(movementInitiationTime);
                }
                else
                {
                    
                    // Update npc timer
                    npcAiComponent.mAiTimer->Update(dt);
                    
                    if (npcAiComponent.mAiTimer->HasTicked())
                    {
                        auto& directionComponent  = mWorld.GetComponent<DirectionComponent>(entityId);
                        
                        const auto newDirection       = static_cast<Direction>(math::RandomInt(0, 3));
                        directionComponent.mDirection = newDirection;
                        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(newDirection), renderableComponent);
                        
                        const auto& targetTile = GetNeighborTile(movementStateComponent.mCurrentCoords, newDirection, levelModelComponent.mLevelTilemap);
                        if (targetTile.mTileTrait == TileTrait::NONE)
                        {
                            ResumeCurrentlyPlayingAnimation(animationTimerComponent);
                            movementStateComponent.mMoving = true;
                        }
                        
                        npcAiComponent.mAiTimer = nullptr;
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
