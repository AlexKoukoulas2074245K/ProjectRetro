//
//  OaksLabMovementAndBattleTriggerOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 24/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OaksLabMovementAndBattleTriggerOverworldFlowState.h"
#include "../components/DirectionComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../common/utils/MilestoneUtils.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../utils/PokemonUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string OaksLabMovementAndBattleTriggerOverworldFlowState::TRAINER_BATTLE_MAIN_MUSIC_TRACK_NAME = "trainer_battle";
const std::string OaksLabMovementAndBattleTriggerOverworldFlowState::RIVAL_TRAINER_MUSIC_NAME = "rival_trainer";

const TileCoords OaksLabMovementAndBattleTriggerOverworldFlowState::OAKS_LAB_MOVEMENT_AND_BATTLE_TRIGGER_1_TILE_COORDS = TileCoords(7, 6);

const int OaksLabMovementAndBattleTriggerOverworldFlowState::OAKS_LAB_GARY_LEVEL_INDEX = 11;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OaksLabMovementAndBattleTriggerOverworldFlowState::OaksLabMovementAndBattleTriggerOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)    
{
    if (HasMilestone(milestones::SEEN_OAK_FIRST_TIME, mWorld) && !HasMilestone(milestones::RECEIVED_PIKACHU, mWorld))
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "OAK: Hey! Don't go#away yet!", mWorld);
    }
    else
    {
        const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
        const auto playerEntityId = GetPlayerEntityId(mWorld);

        auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(playerEntityId);
        auto& playerDirectionComponent     = mWorld.GetComponent<DirectionComponent>(playerEntityId);
        auto& playerRenderableComponent    = mWorld.GetComponent<RenderableComponent>(playerEntityId);
        
        mIsPlayerOnLeftTile = playerMovementStateComponent.mCurrentCoords == OAKS_LAB_MOVEMENT_AND_BATTLE_TRIGGER_1_TILE_COORDS;

        playerDirectionComponent.mDirection = Direction::NORTH;
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(playerDirectionComponent.mDirection), playerRenderableComponent);
        
        const auto garyEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_LEVEL_INDEX, mWorld);

        auto& garyDirectionComponent = mWorld.GetComponent<DirectionComponent>(garyEntityId);
        auto& garyRenderableComponent = mWorld.GetComponent<RenderableComponent>(garyEntityId);

        garyDirectionComponent.mDirection = Direction::SOUTH;
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(garyDirectionComponent.mDirection), garyRenderableComponent);

        QueueDialogForChatbox
        (
            CreateChatbox(mWorld),
            playerStateComponent.mRivalName.GetString() + ": Wait#" + playerStateComponent.mPlayerTrainerName.GetString() + 
            "!#Let's check out#our POK^MON!#@Come on, I'll take#you on!", 
            mWorld
        );

        CreateGaryPathToPlayer();

        SoundService::GetInstance().PlayMusic(RIVAL_TRAINER_MUSIC_NAME, false);

        mEventState = EventState::WAITING_FOR_GARY_TO_REACH_PLAYER;
    }
}

void OaksLabMovementAndBattleTriggerOverworldFlowState::VUpdate(const float)
{    
    if (HasMilestone(milestones::SEEN_OAK_FIRST_TIME, mWorld) && !HasMilestone(milestones::RECEIVED_PIKACHU, mWorld))
    {
        if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
        {
            const auto playerEntityId = GetPlayerEntityId(mWorld);

            auto& playerMovementComponent   = mWorld.GetComponent<MovementStateComponent>(playerEntityId);
            auto& playerDirectionComponent  = mWorld.GetComponent<DirectionComponent>(playerEntityId);
            auto& playerRenderableComponent = mWorld.GetComponent<RenderableComponent>(playerEntityId);

            playerMovementComponent.mMoving     = true;
            playerDirectionComponent.mDirection = Direction::NORTH;
            ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(playerDirectionComponent.mDirection), playerRenderableComponent);

            CompleteOverworldFlow();
        }
    }
    else
    {
        switch (mEventState)
        {
            case EventState::WAITING_FOR_GARY_TO_REACH_PLAYER: UpdateWaitForGaryToReachPlayer(); break;
            case EventState::WAITING_FOR_BATTLE_TO_FINISH:     UpdateWaitForBattleToFinish(); break;
        }        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void OaksLabMovementAndBattleTriggerOverworldFlowState::UpdateWaitForGaryToReachPlayer()
{
    const auto garyEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_LEVEL_INDEX, mWorld);
    auto& garyAiComponent = mWorld.GetComponent<NpcAiComponent>(garyEntityId);

    if (garyAiComponent.mScriptedPathIndex == -1)
    {        
        auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
        auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
        
        playerStateComponent.mLastNpcLevelIndexSpokenTo = OAKS_LAB_GARY_LEVEL_INDEX;
        
        encounterStateComponent.mActiveEncounterType = EncounterType::TRAINER;
        encounterStateComponent.mOpponentTrainerSpeciesName = StringId("RIVAL_1");
        encounterStateComponent.mOpponentTrainerName = playerStateComponent.mRivalName.GetString();
        encounterStateComponent.mOpponentTrainerDefeatedText = playerStateComponent.mRivalName.GetString() + ": WHAT?#Unbelievable!#I picked the#wrong POK^MON!#+END";
        encounterStateComponent.mIsGymLeaderBattle = false;        
        encounterStateComponent.mActivePlayerPokemonRosterIndex = 0;
        encounterStateComponent.mActiveOpponentPokemonRosterIndex = 0;
        encounterStateComponent.mOpponentPokemonRoster.clear();
        encounterStateComponent.mOpponentPokemonRoster.push_back(CreatePokemon
        (
            StringId("EEVEE"),
            5,
            true,
            mWorld
        ));

        SoundService::GetInstance().PlayMusic(TRAINER_BATTLE_MAIN_MUSIC_TRACK_NAME, false);

        mEventState = EventState::WAITING_FOR_BATTLE_TO_FINISH;
    }
}

void OaksLabMovementAndBattleTriggerOverworldFlowState::UpdateWaitForBattleToFinish()
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    if (encounterStateComponent.mActiveEncounterType == EncounterType::NONE)
    {
        SetMilestone(milestones::FIRST_RIVAL_BATTLE_FINSIHED, mWorld);
    }
}

void OaksLabMovementAndBattleTriggerOverworldFlowState::CreateGaryPathToPlayer()
{
    const auto garyEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_LEVEL_INDEX, mWorld);

    auto& garyAiComponent    = mWorld.GetComponent<NpcAiComponent>(garyEntityId);
    garyAiComponent.mAiTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);

    if (mIsPlayerOnLeftTile)
    {
        garyAiComponent.mScriptedPathTileCoords.emplace_back(7, 9);
        garyAiComponent.mScriptedPathTileCoords.emplace_back(7, 7);
    }
    else
    {
        garyAiComponent.mScriptedPathTileCoords.emplace_back(8, 9);
        garyAiComponent.mScriptedPathTileCoords.emplace_back(8, 7);
    }

    garyAiComponent.mScriptedPathIndex = 0;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
