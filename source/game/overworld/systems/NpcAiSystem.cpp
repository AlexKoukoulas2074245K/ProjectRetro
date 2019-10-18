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
#include "../utils/OverworldCharacterLoadingUtils.h"
#include "../utils/OverworldUtils.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/PokemonUtils.h"
#include "../../common/utils/StringUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../common/utils/TrainerUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string NpcAiSystem::GYM_BATTLE_MAIN_MUSIC_TRACK_NAME     = "gym_battle";
const std::string NpcAiSystem::TRAINER_BATTLE_MAIN_MUSIC_TRACK_NAME = "trainer_battle";

const float NpcAiSystem::DYNAMIC_NPC_MIN_MOVEMENT_INITIATION_TIME = 0.5f;
const float NpcAiSystem::DYNAMIC_NPC_MAX_MOVEMENT_INITIATION_TIME = 3.0f;
const float NpcAiSystem::EXCLAMATION_MARK_LIFE_TIME               = 1.0f;

const int NpcAiSystem::TRAINER_LINE_OF_SIGHT      = 3;
const int NpcAiSystem::EXCLAMATION_MARK_ATLAS_COL = 7;
const int NpcAiSystem::EXCLAMATION_MARK_ATLAS_ROW = 46;

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
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& activeEntities = mWorld.GetActiveEntities();
    
    for (const auto& entityId: activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {
            if
            (
                GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID ||
                encounterStateComponent.mActiveEncounterType != EncounterType::NONE
            )
            {
                auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
                auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);

                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                continue;
            }
            
            const auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(entityId);
            
            if (npcAiComponent.mScriptedPathIndex != -1)
            {
                UpdateScriptedPathMovement(dt, entityId);
            }
            else if (npcAiComponent.mIsTrainer)
            {
                UpdateTrainerNpc(dt, entityId);
            }
            else if (npcAiComponent.mMovementType == CharacterMovementType::STATIONARY)
            {
                UpdateStationaryNpc(dt, entityId);
            }
            else if (npcAiComponent.mMovementType == CharacterMovementType::DYNAMIC)
            {
                UpdateDynamicNpc(dt, entityId);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void NpcAiSystem::UpdateScriptedPathMovement(const float, const ecs::EntityId npcEntityId) const
{
    auto& npcAiComponent          = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    auto& movementStateComponent  = mWorld.GetComponent<MovementStateComponent>(npcEntityId);
    auto& directionComponent      = mWorld.GetComponent<DirectionComponent>(npcEntityId);
    auto& renderableComponent     = mWorld.GetComponent<RenderableComponent>(npcEntityId);
    auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(npcEntityId);
    
    const auto& nextTargetTileCoords = npcAiComponent.mScriptedPathTileCoords[npcAiComponent.mScriptedPathIndex];
    
    if (movementStateComponent.mCurrentCoords == nextTargetTileCoords)
    {
        movementStateComponent.mMoving = false;
        PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
        
        if (++npcAiComponent.mScriptedPathIndex >= static_cast<int>(npcAiComponent.mScriptedPathTileCoords.size()))
        {
            npcAiComponent.mScriptedPathTileCoords.clear();
            npcAiComponent.mScriptedPathIndex = -1;
        }
    }
    else
    {
        if (nextTargetTileCoords.mCol < movementStateComponent.mCurrentCoords.mCol)
        {
            directionComponent.mDirection = Direction::WEST;
        }
        else if (nextTargetTileCoords.mCol > movementStateComponent.mCurrentCoords.mCol)
        {
            directionComponent.mDirection = Direction::EAST;
        }
        else if (nextTargetTileCoords.mRow < movementStateComponent.mCurrentCoords.mRow)
        {
            directionComponent.mDirection = Direction::SOUTH;
        }
        else
        {
            directionComponent.mDirection = Direction::NORTH;
        }
        
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(directionComponent.mDirection), renderableComponent);
        ResumeCurrentlyPlayingAnimation(animationTimerComponent);
        movementStateComponent.mMoving = true;
    }
}

void NpcAiSystem::UpdateTrainerNpc(const float dt, const ecs::EntityId npcEntityId) const
{
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& levelModelComponent  = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
    
    auto& npcAiComponent          = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    auto& movementStateComponent  = mWorld.GetComponent<MovementStateComponent>(npcEntityId);
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    // Defeated Npcs behave exactly like stationary npcs
    if (npcAiComponent.mIsDefeated)
    {
        UpdateStationaryNpc(dt, npcEntityId);
        return;
    }
    
    // Npc Exclamation Mark flow
    if (npcAiComponent.mExclamationMarkEntityId != ecs::NULL_ENTITY_ID)
    {
        npcAiComponent.mAiTimer->Update(dt);
        if (npcAiComponent.mAiTimer->HasTicked())
        {
            DestroyExclamationMarkEntity(npcEntityId);
            movementStateComponent.mMoving = true;
        }
        
        return;
    }
    
    // Npc Engaged in combat flow
    if (npcAiComponent.mIsEngagedInCombat)
    {
        const auto& directionComponent = mWorld.GetComponent<DirectionComponent>(npcEntityId);
        const auto& targetTile         = GetNeighborTile(movementStateComponent.mCurrentCoords, directionComponent.mDirection, levelModelComponent.mLevelTilemap);
        
        auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(npcEntityId);
        auto& renderableComponent     = mWorld.GetComponent<RenderableComponent>(npcEntityId);
        
        if
        (
            targetTile.mTileOccupierEntityId != ecs::NULL_ENTITY_ID &&
            targetTile.mTileOccupierEntityId != npcEntityId &&
            movementStateComponent.mMoving == false
        )
        {
            if (playerStateComponent.mLastNpcLevelIndexSpokenTo == npcAiComponent.mLevelIndex)
            {
                StartEncounter(npcEntityId);
            }
            else
            {
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                
                QueueDialogForChatbox(CreateChatbox(mWorld), npcAiComponent.mDialog, mWorld);
                
                playerStateComponent.mLastNpcLevelIndexSpokenTo = npcAiComponent.mLevelIndex;
            }
        }
        else
        {
            ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            movementStateComponent.mMoving = true;
        }
        
        return;
    }
    
    // Npc not engaged in combat flow if not gym leader
    if (npcAiComponent.mIsGymLeader == false)
    {
        CheckForPlayerDistanceAndEncounterEngagement(npcEntityId);
    }
}

void NpcAiSystem::UpdateStationaryNpc(const float dt, const ecs::EntityId entityId) const
{
    // Reset direction to initial one after interacting with player
    const auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(entityId);
    
    npcAiComponent.mAiTimer->Update(dt);
    if (npcAiComponent.mAiTimer->HasTicked())
    {
        auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
        auto& directionComponent  = mWorld.GetComponent<DirectionComponent>(entityId);
        
        directionComponent.mDirection = npcAiComponent.mInitDirection;
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(npcAiComponent.mInitDirection), renderableComponent);
    }
}

void NpcAiSystem::UpdateDynamicNpc(const float dt, const ecs::EntityId entityId) const
{
    auto& movementStateComponent  = mWorld.GetComponent<MovementStateComponent>(entityId);
    auto& npcAiComponent          = mWorld.GetComponent<NpcAiComponent>(entityId);
    auto& animationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(entityId);
    auto& renderableComponent     = mWorld.GetComponent<RenderableComponent>(entityId);
    
    if (movementStateComponent.mMoving)
    {
        return;
    }
    
    // Pause animation timer (pause movement animation)
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
            const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
            const auto& levelModelComponent  = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
            
            auto& directionComponent = mWorld.GetComponent<DirectionComponent>(entityId);
            
            // Pick new direction
            const auto newDirection       = static_cast<Direction>(math::RandomInt(0, 3));
            directionComponent.mDirection = newDirection;
            ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(newDirection), renderableComponent);
            
            const auto& targetTile = GetNeighborTile(movementStateComponent.mCurrentCoords, newDirection, levelModelComponent.mLevelTilemap);
            
            // Start moving if target tile is free
            if (targetTile.mTileTrait == TileTrait::NONE)
            {
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
                movementStateComponent.mMoving = true;
            }
            
            npcAiComponent.mAiTimer = nullptr;
        }
    }
}

void NpcAiSystem::StartEncounter(const ecs::EntityId npcEntityId) const
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    auto opponentName = StringId(npcAiComponent.mTrainerName);
    if (StringStartsWith(npcAiComponent.mTrainerName, "RIVAL"))
    {
        opponentName = playerStateComponent.mRivalName.GetString();
    }

    encounterStateComponent.mActiveEncounterType = EncounterType::TRAINER;
    encounterStateComponent.mOpponentTrainerSpeciesName       = npcAiComponent.mTrainerName;
    encounterStateComponent.mOpponentTrainerName              = opponentName;
    encounterStateComponent.mOpponentTrainerDefeatedText      = npcAiComponent.mSideDialogs[0];
    encounterStateComponent.mIsGymLeaderBattle                = npcAiComponent.mIsGymLeader;
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

    SoundService::GetInstance().PlayMusic
    (
        encounterStateComponent.mIsGymLeaderBattle ?
        GYM_BATTLE_MAIN_MUSIC_TRACK_NAME :
        TRAINER_BATTLE_MAIN_MUSIC_TRACK_NAME,
        false
    );
}

void NpcAiSystem::CheckForPlayerDistanceAndEncounterEngagement(const ecs::EntityId npcEntityId) const
{
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    
    const auto& npcMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(npcEntityId);
    const auto& npcDirectionComponent     = mWorld.GetComponent<DirectionComponent>(npcEntityId);
    
    auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(playerEntityId);
    auto& npcAiComponent               = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    
    // Check for all tiles in line of sight of npc
    for (int i = 1; i <= TRAINER_LINE_OF_SIGHT; ++i)
    {
        const auto& tileCoords = GetNeighborTileCoords(npcMovementStateComponent.mCurrentCoords, npcDirectionComponent.mDirection, i);
        if (playerMovementStateComponent.mCurrentCoords == tileCoords)
        {
            playerMovementStateComponent.mMoving = false;
            npcAiComponent.mIsEngagedInCombat = true;
            
            CreateExclamationMarkEntity(npcEntityId);
            
            const auto trainerMusicTrackName = GetTrainerInfo(npcAiComponent.mTrainerName, mWorld).mTrainerMusicTrackName;
            if (trainerMusicTrackName != StringId())
            {
                SoundService::GetInstance().PlayMusic(trainerMusicTrackName, false);
            }

            break;
        }
    }
}

void NpcAiSystem::CreateExclamationMarkEntity(const ecs::EntityId npcEntityId) const
{
    const auto& npcTransformComponent = mWorld.GetComponent<TransformComponent>(npcEntityId);
    auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    
    npcAiComponent.mAiTimer = std::make_unique<Timer>(EXCLAMATION_MARK_LIFE_TIME);
    
    npcAiComponent.mExclamationMarkEntityId = mWorld.CreateEntity();
    
    auto exclamationMarkRenderableComponent = CreateRenderableComponentForSprite
    (
        CharacterSpriteData
        (
            CharacterMovementType::STATIC,
            EXCLAMATION_MARK_ATLAS_COL,
            EXCLAMATION_MARK_ATLAS_ROW
        )
    );
    
    ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::SOUTH), *exclamationMarkRenderableComponent);
    
    auto exclamationMarkTransformComponent       = std::make_unique<TransformComponent>();
    exclamationMarkTransformComponent->mPosition = npcTransformComponent.mPosition;
    exclamationMarkTransformComponent->mRotation = npcTransformComponent.mRotation;
    exclamationMarkTransformComponent->mScale    = npcTransformComponent.mScale;
    
    exclamationMarkTransformComponent->mPosition.y += GAME_TILE_SIZE;
    
    mWorld.AddComponent<RenderableComponent>(npcAiComponent.mExclamationMarkEntityId, std::move(exclamationMarkRenderableComponent));
    mWorld.AddComponent<TransformComponent>(npcAiComponent.mExclamationMarkEntityId, std::move(exclamationMarkTransformComponent));
}

void NpcAiSystem::DestroyExclamationMarkEntity(const ecs::EntityId npcEntityId) const
{
    auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    
    npcAiComponent.mAiTimer = nullptr;
    
    mWorld.DestroyEntity(npcAiComponent.mExclamationMarkEntityId);
    
    npcAiComponent.mExclamationMarkEntityId = ecs::NULL_ENTITY_ID;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
