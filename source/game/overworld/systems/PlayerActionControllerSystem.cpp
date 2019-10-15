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
#include "MovementControllerSystem.h"
#include "../components/WarpConnectionsSingletonComponent.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/components/PlayerTagComponent.h"
#include "../../common/flowstates/MainMenuOverworldFlowState.h"
#include "../../common/utils/MilestoneUtils.h"
#include "../../common/utils/PokemonUtils.h"
#include "../../common/utils/PokemonItemsUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../overworld/OverworldConstants.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/LevelModelComponent.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/components/NpcAiComponent.h"
#include "../../overworld/components/TransitionAnimationStateSingletonComponent.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PlayerActionControllerSystem::MENU_OPEN_SFX_NAME    = "general/main_menu_open";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerActionControllerSystem::PlayerActionControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask
    <
        AnimationTimerComponent, 
        DirectionComponent, 
        PlayerTagComponent, 
        RenderableComponent, 
        MovementStateComponent
    >();
}

void PlayerActionControllerSystem::VUpdateAssociatedComponents(const float) const
{    
    const auto& warpConnectionsComponent = mWorld.GetSingletonComponent<WarpConnectionsSingletonComponent>();
    const auto& transitionStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    auto& encounterStateComponent        = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& inputStateComponent            = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    
	StartPendingPostEncounterConversation();
    AddPendingItemsToBag();
    
    for (const auto& entityId : mWorld.GetActiveEntities())
    {
        if (ShouldProcessEntity(entityId))
        {
            auto& animationTimerComponent  = mWorld.GetComponent<AnimationTimerComponent>(entityId);
            auto& movementStateComponent   = mWorld.GetComponent<MovementStateComponent>(entityId);
            auto& renderableComponent      = mWorld.GetComponent<RenderableComponent>(entityId);
            
            if (movementStateComponent.mMoving)
            {
                continue;
            }
            
            if
            (
                encounterStateComponent.mActiveEncounterType != EncounterType::NONE ||
                transitionStateComponent.mIsPlayingTransitionAnimation ||
                warpConnectionsComponent.mHasPendingWarpConnection ||
                GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID ||
                IsAnyOverworldFlowCurrentlyRunning(mWorld)
            )
            {
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                continue;
            }
            
            if (IsAnyNpcEngagedInCombat(mWorld))
            {
                movementStateComponent.mMoving = false;
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                continue;
            }
            
            if (inputStateComponent.mHasBeenConsumed)
            {
                continue;
            }
                        
            // We'll set the movement direction and intent of movement (mMoving) 
            // in this system's udpate, and do the checks (and possibly revert the intent) in MovementControllerSystem
            auto& directionComponent = mWorld.GetComponent<DirectionComponent>(entityId);
            
            if (inputStateComponent.mCurrentInputState.at(VirtualActionType::START_BUTTON) == VirtualActionInputState::TAPPED)
            {
                SoundService::GetInstance().PlaySfx(MENU_OPEN_SFX_NAME);
                StartOverworldFlowState<MainMenuOverworldFlowState>(mWorld);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::A_BUTTON) == VirtualActionInputState::TAPPED)
            {
                CheckForNpcInteraction(directionComponent.mDirection, movementStateComponent);
                if (GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID)
                {
                    PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
                }
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT_ARROW) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::WEST, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::LEFT_ARROW) == VirtualActionInputState::PRESSED)
            {
                movementStateComponent.mMoving = true;
                ChangePlayerDirectionAndAnimation(Direction::WEST, renderableComponent, directionComponent);
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT_ARROW) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::EAST, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::RIGHT_ARROW) == VirtualActionInputState::PRESSED)
            {
                movementStateComponent.mMoving = true;
                ChangePlayerDirectionAndAnimation(Direction::EAST, renderableComponent, directionComponent);
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP_ARROW) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::NORTH, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::UP_ARROW) == VirtualActionInputState::PRESSED)
            {
                movementStateComponent.mMoving = true;
                ChangePlayerDirectionAndAnimation(Direction::NORTH, renderableComponent, directionComponent);
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN_ARROW) == VirtualActionInputState::TAPPED)
            {
                ChangePlayerDirectionAndAnimation(Direction::SOUTH, renderableComponent, directionComponent);
            }
            else if (inputStateComponent.mCurrentInputState.at(VirtualActionType::DOWN_ARROW) == VirtualActionInputState::PRESSED)
            {
                movementStateComponent.mMoving = true;
                ChangePlayerDirectionAndAnimation(Direction::SOUTH, renderableComponent, directionComponent);
                ResumeCurrentlyPlayingAnimation(animationTimerComponent);
            }
            // All movement keys released and is currently not moving to another tile
            else
            {
                PauseAndResetCurrentlyPlayingAnimation(animationTimerComponent, renderableComponent);
            }

            inputStateComponent.mHasBeenConsumed = true;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PlayerActionControllerSystem::StartPendingPostEncounterConversation() const
{
	auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

	// If just defeated gym leader force show chatbox with follow up tm text
	if (playerStateComponent.mJustDefeatedGymLeader)
	{
		playerStateComponent.mJustDefeatedGymLeader = false;

		const auto& gymLeaderNpcAiComponent = mWorld.GetComponent<NpcAiComponent>
		(
			GetNpcEntityIdFromLevelIndex
			(
				playerStateComponent.mLastNpcLevelIndexSpokenTo,
				mWorld
			)
		);

		QueueDialogForChatbox(CreateChatbox(mWorld), gymLeaderNpcAiComponent.mSideDialogs[2], mWorld);
	}
}

void PlayerActionControllerSystem::AddPendingItemsToBag() const
{
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();

    auto& levelModelComponent  = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
    auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    if (playerStateComponent.mPendingItemToBeAdded != StringId())
    {		
		// Milestone handling
		if (GetItemStats(playerStateComponent.mPendingItemToBeAdded, mWorld).mEffect == StringId("BADGE"))
		{
			return;
		}
        else if (playerStateComponent.mPendingItemToBeAdded == OAKS_PARCEL_ITEM_NAME)
        {
            SetMilestone(milestones::RECEIVED_OAKS_PARCEL, mWorld);

			if (playerStateComponent.mPendingItemToBeAddedDiscoveryType == ItemDiscoveryType::DELIVERED)
			{
				RemoveItemFromBag(playerStateComponent.mPendingItemToBeAdded, mWorld);
			}
			else
			{
				AddItemToBag(playerStateComponent.mPendingItemToBeAdded, mWorld);
			}
            playerStateComponent.mPendingItemToBeAdded = StringId();
            playerStateComponent.mPendingItemToBeAddedDiscoveryType = ItemDiscoveryType::NO_ITEM;
            return;
        }
		else if (playerStateComponent.mPendingItemToBeAdded == POKEDEX_ITEM_NAME)
		{
			SetMilestone(milestones::RECEIVED_POKEDEX, mWorld);
			playerStateComponent.mPendingItemToBeAdded = StringId();
			playerStateComponent.mPendingItemToBeAddedDiscoveryType = ItemDiscoveryType::NO_ITEM;
			return;
		}

		if (playerStateComponent.mPendingItemToBeAddedDiscoveryType == ItemDiscoveryType::DELIVERED)
		{
			RemoveItemFromBag(playerStateComponent.mPendingItemToBeAdded, mWorld);
		}
		else
		{
			AddItemToBag(playerStateComponent.mPendingItemToBeAdded, mWorld);
		}
                        
        const auto npcEntityId = GetNpcEntityIdFromLevelIndex(playerStateComponent.mLastNpcLevelIndexSpokenTo, mWorld);
        auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);        

        // Found item (pokeball npc)
        if (playerStateComponent.mPendingItemToBeAddedDiscoveryType == ItemDiscoveryType::FOUND)
        {
            playerStateComponent.mCollectedNpcItemEntries.emplace_back
            (
                playerStateComponent.mLastOverworldLevelName,
                playerStateComponent.mLastNpcLevelIndexSpokenTo
            );

            const auto playerEntityId                = GetPlayerEntityId(mWorld);
            const auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(playerEntityId);
            const auto& playerDirectionComponent     = mWorld.GetComponent<DirectionComponent>(playerEntityId);            

            auto& npcItemTile = GetNeighborTile
            (
                playerMovementStateComponent.mCurrentCoords, 
                playerDirectionComponent.mDirection,
                levelModelComponent.mLevelTilemap
            );

            mWorld.DestroyEntity(npcEntityId);
            playerStateComponent.mLastNpcLevelIndexSpokenTo = -1;
            npcItemTile.mTileOccupierType = TileOccupierType::NONE;
            npcItemTile.mTileOccupierEntityId = ecs::NULL_ENTITY_ID;            
        }
        // Collected item 
        else if 
		(
			playerStateComponent.mPendingItemToBeAddedDiscoveryType == ItemDiscoveryType::GOT ||
			playerStateComponent.mPendingItemToBeAddedDiscoveryType == ItemDiscoveryType::RECEIVED
		)
        {
            if (npcAiComponent.mSideDialogs.size() > 0)
            {
				if (npcAiComponent.mIsGymLeader == false)
				{
					npcAiComponent.mDialog = npcAiComponent.mSideDialogs[0];
				}
                
                playerStateComponent.mCollectedItemNonDestructibleNpcEntries.emplace_back
                (
                    activeLevelComponent.mActiveLevelNameId,
                    npcAiComponent.mLevelIndex
                );
            }
        }  

        playerStateComponent.mPendingItemToBeAdded = StringId();
        playerStateComponent.mPendingItemToBeAddedDiscoveryType = ItemDiscoveryType::NO_ITEM;
    }
}

void PlayerActionControllerSystem::CheckForNpcInteraction
(
    const Direction direction,
    const MovementStateComponent& movementStateComponent
) const
{
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& levelModelComponent  = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
    const auto& tile = GetNeighborTile(movementStateComponent.mCurrentCoords, direction, levelModelComponent.mLevelTilemap);
    
    if (tile.mTileOccupierType == TileOccupierType::NPC)
    {
        const auto& npcMovementState = mWorld.GetComponent<MovementStateComponent>(tile.mTileOccupierEntityId);
        
        // Disallow talking to moving npc
        if (npcMovementState.mMoving == false)
        {
            auto& npcAiComponent         = mWorld.GetComponent<NpcAiComponent>(tile.mTileOccupierEntityId);
            auto& playerStateComponent   = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
            auto& npcDirectionComponent  = mWorld.GetComponent<DirectionComponent>(tile.mTileOccupierEntityId);            
            
            const auto newNpcDirection       = GetDirectionFacingDirection(direction);
            npcDirectionComponent.mDirection = newNpcDirection;            
            
            if (mWorld.HasComponent<RenderableComponent>(tile.mTileOccupierEntityId) && npcAiComponent.mMovementType != CharacterMovementType::STATIC)
            {
                auto& npcRenderableComponent = mWorld.GetComponent<RenderableComponent>(tile.mTileOccupierEntityId);
                ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(newNpcDirection), npcRenderableComponent);
            }
            
            QueueDialogForChatbox(CreateChatbox(mWorld), npcAiComponent.mDialog, mWorld);
           
            playerStateComponent.mLastNpcLevelIndexSpokenTo = GetNpcLevelIndexFromEntityId(tile.mTileOccupierEntityId, mWorld);
            npcAiComponent.mAiTimer->Reset();
            
            if (npcAiComponent.mIsTrainer && npcAiComponent.mIsDefeated == false)
            {
                npcAiComponent.mIsEngagedInCombat = true;
            }
        }
    }
}

void PlayerActionControllerSystem::ChangePlayerDirectionAndAnimation
(
    const Direction direction,
    RenderableComponent& playerRenderableComponent,
    DirectionComponent& playerDirectionComponent
) const
{
    playerDirectionComponent.mDirection = direction;
    ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(direction), playerRenderableComponent);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
