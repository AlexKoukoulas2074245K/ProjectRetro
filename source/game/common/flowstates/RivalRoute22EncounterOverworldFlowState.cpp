//
//  RivalRoute22EncounterOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 15/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "RivalRoute22EncounterOverworldFlowState.h"
#include "../components/DirectionComponent.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/components/NpcAiComponent.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../overworld/utils/OverworldCharacterLoadingUtils.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string RivalRoute22EncounterOverworldFlowState::RIVAL_TRAINER_MUSIC_NAME = "rival_trainer";
const std::string RivalRoute22EncounterOverworldFlowState::LEVEL_MUSIC_NAME         = "route22";

const TileCoords RivalRoute22EncounterOverworldFlowState::EXCLAMATION_MARK_ATLAS_COORDS = TileCoords(7, 46);
const TileCoords RivalRoute22EncounterOverworldFlowState::RIVAL_ENTRANCE_COORDS         = TileCoords(36, 19);
const TileCoords RivalRoute22EncounterOverworldFlowState::RIVAL_SPEECH_COORDS_1         = TileCoords(41, 19);
const TileCoords RivalRoute22EncounterOverworldFlowState::RIVAL_SPEECH_COORDS_2         = TileCoords(42, 19);
const TileCoords RivalRoute22EncounterOverworldFlowState::RIVAL_EXIT_COORDS_1           = TileCoords(44, 19);
const TileCoords RivalRoute22EncounterOverworldFlowState::RIVAL_EXIT_COORDS_2           = TileCoords(44, 14);

const int RivalRoute22EncounterOverworldFlowState::RIVAL_1_LEVEL_INDEX = 0;

const float RivalRoute22EncounterOverworldFlowState::EXCLAMATION_MARK_LIFE_TIME = 1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

RivalRoute22EncounterOverworldFlowState::RivalRoute22EncounterOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mExclamationMarkTimer(EXCLAMATION_MARK_LIFE_TIME)
    , mExclamationMarkEntityId(ecs::NULL_ENTITY_ID)    
    , mEventState(EventState::EXCLAMATION_MARK)
{
    const auto& playerMovementState = mWorld.GetComponent<MovementStateComponent>(GetPlayerEntityId(mWorld));

    if 
    (
        HasMilestone(milestones::RECEIVED_POKEDEX, mWorld) && 
        !HasMilestone(milestones::BOULDERBADGE, mWorld) && 
        !HasMilestone(milestones::FIRST_RIVAL_BATTLE_WON, mWorld))
    {
        mIsPlayerOnBottomTile = playerMovementState.mCurrentCoords == TileCoords(RIVAL_SPEECH_COORDS_2.mCol, RIVAL_SPEECH_COORDS_2.mRow);
        CreateExlamationMark();
    }
    else
    {        
        CompleteOverworldFlow();
    }
}

void RivalRoute22EncounterOverworldFlowState::VUpdate(const float dt)
{
    switch (mEventState)
    {
        case EventState::EXCLAMATION_MARK:      UpdateExclamationMark(dt); break;
        case EventState::RIVAL_ENTRANCE:        UpdateRivalEntrance(); break;
        case EventState::RIVAL_WAIT_FOR_BATTLE: UpdateWaitForRivalBattleToEnd(); break;
        case EventState::RIVAL_DEFEATED_DIALOG: UpdateRivalDefeatedDialog(); break;
        case EventState::RIVAL_EXIT:            UpdateRivalExit(); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void RivalRoute22EncounterOverworldFlowState::UpdateExclamationMark(const float dt)
{
    mExclamationMarkTimer.Update(dt);
    if (mExclamationMarkTimer.HasTicked())
    {
        mWorld.DestroyEntity(mExclamationMarkEntityId);
        SoundService::GetInstance().PlayMusic(RIVAL_TRAINER_MUSIC_NAME, false);
        PositionRivalSprite();
        CreateScriptedPath(true);

        mEventState = EventState::RIVAL_ENTRANCE;
    }
}

void RivalRoute22EncounterOverworldFlowState::UpdateRivalEntrance()
{
    const auto rivalEntityId = GetNpcEntityIdFromLevelIndex(RIVAL_1_LEVEL_INDEX, mWorld);

    auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(rivalEntityId);

    if (npcAiComponent.mScriptedPathIndex == -1)
    {
        if (mIsPlayerOnBottomTile == false)
        {
            const auto playerEntityId = GetPlayerEntityId(mWorld);

            auto& playerRenderableComponent = mWorld.GetComponent<RenderableComponent>(GetPlayerEntityId(mWorld));
            auto& rivalRenderableComponent  = mWorld.GetComponent<RenderableComponent>(rivalEntityId);

            auto& playerDirectionComponent = mWorld.GetComponent<DirectionComponent>(playerEntityId);
            auto& rivalDirectionComponent  = mWorld.GetComponent<DirectionComponent>(rivalEntityId);

            playerDirectionComponent.mDirection = Direction::SOUTH;
            rivalDirectionComponent.mDirection  = Direction::NORTH;

            ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::SOUTH), playerRenderableComponent);
            ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::NORTH), rivalRenderableComponent);
        }

        npcAiComponent.mIsEngagedInCombat = true;

        mEventState = EventState::RIVAL_WAIT_FOR_BATTLE;
    }
}

void RivalRoute22EncounterOverworldFlowState::UpdateWaitForRivalBattleToEnd()
{    
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    if (playerStateComponent.mRivalBattleJustEnded)
    {
        if (playerStateComponent.mLastBattleWon)
        {
            SetMilestone(milestones::FIRST_RIVAL_BATTLE_WON, mWorld);
            
            const auto rivalEntityId = GetNpcEntityIdFromLevelIndex(RIVAL_1_LEVEL_INDEX, mWorld);
            auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(rivalEntityId);

            QueueDialogForChatbox(CreateChatbox(mWorld), npcAiComponent.mSideDialogs[1], mWorld);

            mEventState = EventState::RIVAL_DEFEATED_DIALOG;
        }
        else
        {
            CompleteOverworldFlow();
        }
    }
}

void RivalRoute22EncounterOverworldFlowState::UpdateRivalDefeatedDialog()
{
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        CreateScriptedPath(false);
        SoundService::GetInstance().PlayMusic(RIVAL_TRAINER_MUSIC_NAME, false);
        mEventState = EventState::RIVAL_EXIT;
    }
}

void RivalRoute22EncounterOverworldFlowState::UpdateRivalExit()
{
    const auto rivalEntityId = GetNpcEntityIdFromLevelIndex(RIVAL_1_LEVEL_INDEX, mWorld);

    auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(rivalEntityId);

    if (npcAiComponent.mScriptedPathIndex == -1)
    {
        SoundService::GetInstance().PlayMusic(LEVEL_MUSIC_NAME, false);
        DestroyOverworldNpcEntityAndEraseTileInfo(rivalEntityId, mWorld);
        CompleteOverworldFlow();
    }
}

void RivalRoute22EncounterOverworldFlowState::CreateExlamationMark()
{            
    mExclamationMarkEntityId = mWorld.CreateEntity();

    auto exclamationMarkRenderableComponent = CreateRenderableComponentForSprite
    (
        CharacterSpriteData
        (
            CharacterMovementType::STATIC,
            EXCLAMATION_MARK_ATLAS_COORDS.mCol,
            EXCLAMATION_MARK_ATLAS_COORDS.mRow
        )
    );

    ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::SOUTH), *exclamationMarkRenderableComponent);

    auto exclamationMarkTransformComponent = std::make_unique<TransformComponent>();
    exclamationMarkTransformComponent->mPosition = TileCoordsToPosition(RIVAL_ENTRANCE_COORDS.mCol + 1, RIVAL_ENTRANCE_COORDS.mRow);

    exclamationMarkTransformComponent->mPosition.y += GAME_TILE_SIZE;

    mWorld.AddComponent<RenderableComponent>(mExclamationMarkEntityId, std::move(exclamationMarkRenderableComponent));
    mWorld.AddComponent<TransformComponent>(mExclamationMarkEntityId, std::move(exclamationMarkTransformComponent));
}

void RivalRoute22EncounterOverworldFlowState::PositionRivalSprite()
{
    const auto rivalEntityId = GetNpcEntityIdFromLevelIndex(RIVAL_1_LEVEL_INDEX, mWorld);

    auto& transformComponent = mWorld.GetComponent<TransformComponent>(rivalEntityId);
    transformComponent.mPosition = TileCoordsToPosition(RIVAL_ENTRANCE_COORDS.mCol, RIVAL_ENTRANCE_COORDS.mRow);
    
    auto& movementStateComponent = mWorld.GetComponent<MovementStateComponent>(rivalEntityId);
    movementStateComponent.mCurrentCoords = RIVAL_ENTRANCE_COORDS;

    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    auto& levelModelComponent = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));

    GetTile(RIVAL_ENTRANCE_COORDS.mCol, RIVAL_ENTRANCE_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = rivalEntityId;
    GetTile(RIVAL_ENTRANCE_COORDS.mCol, RIVAL_ENTRANCE_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NPC;
}

void RivalRoute22EncounterOverworldFlowState::CreateScriptedPath(const bool isEnteringScene)
{
    const auto rivalEntityId = GetNpcEntityIdFromLevelIndex(RIVAL_1_LEVEL_INDEX, mWorld);
    auto& rivalAiComponent = mWorld.GetComponent<NpcAiComponent>(rivalEntityId);
    rivalAiComponent.mAiTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);

    if (isEnteringScene)
    {
        if (mIsPlayerOnBottomTile)
        {
            rivalAiComponent.mScriptedPathTileCoords.emplace_back(RIVAL_SPEECH_COORDS_1.mCol, RIVAL_SPEECH_COORDS_1.mRow);
        }
        else
        {
            rivalAiComponent.mScriptedPathTileCoords.emplace_back(RIVAL_SPEECH_COORDS_2.mCol, RIVAL_SPEECH_COORDS_2.mRow);
        }
    }
    else
    {
        rivalAiComponent.mScriptedPathTileCoords.emplace_back(RIVAL_EXIT_COORDS_1.mCol, RIVAL_EXIT_COORDS_1.mRow);
        rivalAiComponent.mScriptedPathTileCoords.emplace_back(RIVAL_EXIT_COORDS_2.mCol, RIVAL_EXIT_COORDS_2.mRow);
    }

    rivalAiComponent.mScriptedPathIndex = 0;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
