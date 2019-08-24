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
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/PokemonUtils.h"
#include "../../common/utils/StringUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/utils/AnimationUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float NpcAiSystem::DYNAMIC_NPC_MIN_MOVEMENT_INITIATION_TIME = 0.5f;
const float NpcAiSystem::DYNAMIC_NPC_MAX_MOVEMENT_INITIATION_TIME = 3.0f;

const int NpcAiSystem::TRAINER_LINE_OF_SIGHT = 3;

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
    
    const auto& activeEntities = mWorld.GetActiveEntities();
    
    for (const auto& entityId: activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {
            if (GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID)
            {
                auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
                auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);

                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                continue;
            }

            auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(entityId);
            
            if (npcAiComponent.mMovementType == CharacterMovementType::STATIONARY)
            {
                if
                (
                    npcAiComponent.mIsTrainer &&
                    npcAiComponent.mIsGymLeader == false &&
                    npcAiComponent.mIsDefeated == false
                )
                {
                    CheckForPlayerDistanceAndEncounterEngagement(entityId);
                }
                else
                {
                    auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
                    animationTimerComponent.mAnimationTimer->Resume();
                    animationTimerComponent.mAnimationTimer->Update(dt);
                    if (animationTimerComponent.mAnimationTimer->HasTicked())
                    {
                        auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
                        auto& directionComponent  = mWorld.GetComponent<DirectionComponent>(entityId);
                        
                        directionComponent.mDirection = npcAiComponent.mInitDirection;
                        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(npcAiComponent.mInitDirection), renderableComponent);
                    }
                }
            }
            else if (npcAiComponent.mMovementType == CharacterMovementType::DYNAMIC)
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

void NpcAiSystem::CheckForPlayerDistanceAndEncounterEngagement(const ecs::EntityId npcEntityId) const
{
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    
    const auto& npcMovementStateComponent    = mWorld.GetComponent<MovementStateComponent>(npcEntityId);
    const auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(playerEntityId);
    const auto& npcDirectionComponent        = mWorld.GetComponent<DirectionComponent>(npcEntityId);
    const auto& npcAiComponent               = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    auto& encounterStateComponent            = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    // Check for all tiles in line of sight of npc
    for (int i = 1; i <= TRAINER_LINE_OF_SIGHT; ++i)
    {
        const auto& tileCoords = GetNeighborTileCoords(npcMovementStateComponent.mCurrentCoords, npcDirectionComponent.mDirection, i);
        if (playerMovementStateComponent.mCurrentCoords == tileCoords)
        {
            encounterStateComponent.mActiveEncounterType = EncounterType::TRAINER;
            encounterStateComponent.mOpponentTrainerSpeciesName       = npcAiComponent.mTrainerName;
            encounterStateComponent.mOpponentTrainerName              = StringId(npcAiComponent.mTrainerName);
            encounterStateComponent.mOpponentTrainerDefeatedText      = npcAiComponent.mSideDialogs[0];
            encounterStateComponent.mActivePlayerPokemonRosterIndex   = 0;
            encounterStateComponent.mActiveOpponentPokemonRosterIndex = 0;
            encounterStateComponent.mOpponentPokemonRoster.clear();
            for (const auto& pokemon: npcAiComponent.mPokemonRoster)
            {
                encounterStateComponent.mOpponentPokemonRoster.push_back(CreatePokemon
                (
                    pokemon->mName,
                    pokemon->mLevel,
                    true,
                    mWorld
                ));
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
