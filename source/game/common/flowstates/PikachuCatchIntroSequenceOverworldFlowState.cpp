//
//  PikachuCatchIntroSequenceOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 15/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PikachuCatchIntroSequenceOverworldFlowState.h"
#include "../components/DirectionComponent.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/PokemonUtils.h"
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

const std::string PikachuCatchIntroSequenceOverworldFlowState::OAK_APPEARS_MUSIC_NAME = "oak_appears";
const std::string PikachuCatchIntroSequenceOverworldFlowState::OAKS_LAB_MUSIC_NAME    = "oaks_lab";
const std::string PikachuCatchIntroSequenceOverworldFlowState::FOLLOW_MUSIC_NAME      = "follow";
const std::string PikachuCatchIntroSequenceOverworldFlowState::WILD_BATTLE_MUSIC_NAME = "wild_battle";

const StringId PikachuCatchIntroSequenceOverworldFlowState::PIKACHU_NAME        = StringId("PIKACHU");
const StringId PikachuCatchIntroSequenceOverworldFlowState::OAKS_LAB_LEVEL_NAME = StringId("in_oaks_lab");

const TileCoords PikachuCatchIntroSequenceOverworldFlowState::EXCLAMATION_MARK_ATLAS_COORDS = TileCoords(7, 46);
const TileCoords PikachuCatchIntroSequenceOverworldFlowState::OAK_ENTRANCE_COORDS           = TileCoords(16, 18);
const TileCoords PikachuCatchIntroSequenceOverworldFlowState::OAK_SPEECH_COORDS_1           = TileCoords(16, 22);
const TileCoords PikachuCatchIntroSequenceOverworldFlowState::OAK_SPEECH_COORDS_2           = TileCoords(17, 22);

const int PikachuCatchIntroSequenceOverworldFlowState::OAKS_LAB_OAK_LEVEL_INDEX = 10;
const int PikachuCatchIntroSequenceOverworldFlowState::OAKS_LAB_GARY_LEVEL_INDEX = 11;
const int PikachuCatchIntroSequenceOverworldFlowState::PALLET_OAK_LEVEL_INDEX = 6;

const int PikachuCatchIntroSequenceOverworldFlowState::PIKACHU_LEVEL   = 5;

const float PikachuCatchIntroSequenceOverworldFlowState::EXCLAMATION_MARK_LIFE_TIME = 1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PikachuCatchIntroSequenceOverworldFlowState::PikachuCatchIntroSequenceOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mTimer(EXCLAMATION_MARK_LIFE_TIME)
    , mExclamationMarkEntityId(ecs::NULL_ENTITY_ID)    
    , mEventState(EventState::EXCLAMATION_MARK)
{
    const auto& playerMovementState = mWorld.GetComponent<MovementStateComponent>(GetPlayerEntityId(mWorld));
    mIsPlayerOnLeftTile = playerMovementState.mCurrentCoords == TileCoords(OAK_SPEECH_COORDS_1.mCol, OAK_SPEECH_COORDS_1.mRow + 1);

    SetMilestone(milestones::SEEN_OAK_FIRST_TIME, mWorld);

    SoundService::GetInstance().PlayMusic(OAK_APPEARS_MUSIC_NAME, false);

    QueueDialogForChatbox(CreateChatbox(mWorld), "OAK: Hey! Wait!#Don't go out!+FREEZE", mWorld);
}

void PikachuCatchIntroSequenceOverworldFlowState::VUpdate(const float dt)
{    
    switch (mEventState)
    {
        case EventState::EXCLAMATION_MARK:                               UpdateExclamationMark(dt); break;
        case EventState::OAK_ENTRANCE:                                   UpdateOakEntrance(); break;
        case EventState::OAK_THAT_WAS_CLOSE_DIALOG:                      UpdateOakThatWasCloseDialog(); break;        
        case EventState::WAIT_FOR_PIKACHU_CAPTURE:                       UpdateWaitForPikachuCapture(); break;
        case EventState::OAK_PHEW_DIALOG:                                UpdateOakPhewDialog(); break;
        case EventState::OAK_TALL_GRASS_DIALOG:                          UpdateOakTallGrassDialog(); break;
        case EventState::FOLLOWING_OAK_TO_LAB:                           UpdateFollowingOakToLab(); break;
        case EventState::OAK_MOVING_IN_LAB:                              UpdateOakMovingInLab(); break;
        case EventState::PLAYER_MOVING_TO_OAK_IN_LAB:                    UpdatePlayerMovingToOakInLab(); break;
        case EventState::GARY_COMPLAINING_CONVERSATION:                  UpdateGaryComplainingConversation(dt); break;
        case EventState::OAK_URGING_PLAYER_TO_TAKE_BALL_CONVERSATION:    UpdateOakUrgingPlayerToTakeBallConversation(dt); break;
        case EventState::GARY_COMPLAINING_AGAIN_CONVERSATION:            UpdateGaryComplainingAgainConversation(dt); break;
        case EventState::OAK_RESPONDING_TO_GARY_COMPLAINTS_CONVERSATION: UpdateOakComplainingToGaryComplaintsConversation(dt); break;
    }   
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PikachuCatchIntroSequenceOverworldFlowState::UpdateExclamationMark(const float dt)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        if (mExclamationMarkEntityId == ecs::NULL_ENTITY_ID)
        {
            CreateExlamationMark();
        }
        else
        {
            mTimer.Update(dt);
            if (mTimer.HasTicked())
            {
                mWorld.DestroyEntity(mExclamationMarkEntityId);         
                DestroyActiveTextbox(mWorld);
                ChangeCharacterDirection(Character::PLAYER, TimelinePoint::PALLET_TOWN_PRE_ENCOUNTER, Direction::SOUTH);
                PositionOakSprite(TimelinePoint::PALLET_TOWN_PRE_ENCOUNTER);
                CreateOakEntranceScriptedPath();

                mEventState = EventState::OAK_ENTRANCE;
            }
        }        
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateOakEntrance()
{    
    const auto oakEntityId = GetNpcEntityIdFromLevelIndex(PALLET_OAK_LEVEL_INDEX, mWorld);

    auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(oakEntityId);

    if (npcAiComponent.mScriptedPathIndex == -1)
    {        
        QueueDialogForChatbox(CreateChatbox(mWorld), "OAK: That was#close!#@Wild POK^MON live#in tall grass!", mWorld);
        mEventState = EventState::OAK_THAT_WAS_CLOSE_DIALOG;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateOakThatWasCloseDialog()
{
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        if (mIsPlayerOnLeftTile)
        {
            ChangeCharacterDirection(Character::OAK, TimelinePoint::PALLET_TOWN_PRE_ENCOUNTER, Direction::EAST);
        }
        else
        {
            ChangeCharacterDirection(Character::OAK, TimelinePoint::PALLET_TOWN_PRE_ENCOUNTER, Direction::WEST);
        }

        auto& encounterStateComponent                = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
        encounterStateComponent.mActiveEncounterType = EncounterType::WILD;
        encounterStateComponent.mIsGymLeaderBattle   = false;
        encounterStateComponent.mOpponentPokemonRoster.push_back
        (
            CreatePokemon
            (
                PIKACHU_NAME,
                PIKACHU_LEVEL,
                false,
                mWorld
            )
        );

        encounterStateComponent.mActivePlayerPokemonRosterIndex   = 0;
        encounterStateComponent.mActiveOpponentPokemonRosterIndex = 0;
        encounterStateComponent.mIsPikachuCaptureFlowActive       = true;

        SoundService::GetInstance().PlayMusic(WILD_BATTLE_MUSIC_NAME, false);

        mEventState = EventState::WAIT_FOR_PIKACHU_CAPTURE;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateWaitForPikachuCapture()
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    if (encounterStateComponent.mActiveEncounterType == EncounterType::NONE)
    {
        encounterStateComponent.mIsPikachuCaptureFlowActive = false;

        PositionOakSprite(TimelinePoint::PALLET_TOWN_POST_ENCOUNTER);
        QueueDialogForChatbox(CreateChatbox(mWorld), "OAK: Whew..", mWorld);
        mEventState = EventState::OAK_PHEW_DIALOG;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateOakPhewDialog()
{    
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        ChangeCharacterDirection(Character::OAK, TimelinePoint::PALLET_TOWN_POST_ENCOUNTER, Direction::NORTH);
        QueueDialogForChatbox
        (
            CreateChatbox(mWorld), 
            "OAK: A POK^MON can#appear anytime in#tall grass.#@You need your own#POK^MON for your#protection.#I know!#@Here, come with#me!", 
            mWorld
        );
        mEventState = EventState::OAK_TALL_GRASS_DIALOG;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateOakTallGrassDialog()
{
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        SoundService::GetInstance().PlayMusic(FOLLOW_MUSIC_NAME, false);
        CreateScriptedPathToLab();
        mEventState = EventState::FOLLOWING_OAK_TO_LAB;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateFollowingOakToLab()
{
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    if (activeLevelComponent.mActiveLevelNameId == OAKS_LAB_LEVEL_NAME)
    {
        PositionOakSprite(TimelinePoint::OAKS_LAB_ENTRY);
        mWorld.RemoveComponent<NpcAiComponent>(GetPlayerEntityId(mWorld));
        CreateOakPathInLab();
        mEventState = EventState::OAK_MOVING_IN_LAB;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateOakMovingInLab()
{
    const auto oakEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_OAK_LEVEL_INDEX, mWorld);

    auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(oakEntityId);

    if (npcAiComponent.mScriptedPathIndex == -1)
    {
        PositionOakSprite(TimelinePoint::OAKS_LAB_FINAL);
        ChangeCharacterDirection(Character::GARY, TimelinePoint::OAKS_LAB_FINAL, Direction::NORTH);
        CreatePlayerMovingToOakPathInLab();
        mEventState = EventState::PLAYER_MOVING_TO_OAK_IN_LAB;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdatePlayerMovingToOakInLab()
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto playerEntityId = GetPlayerEntityId(mWorld);

    auto& playerNpcAiComponent = mWorld.GetComponent<NpcAiComponent>(playerEntityId);
    if (playerNpcAiComponent.mScriptedPathIndex == -1)
    {
        mWorld.RemoveComponent<NpcAiComponent>(playerEntityId);

        SoundService::GetInstance().PlayMusic(OAKS_LAB_MUSIC_NAME, false);

        QueueDialogForChatbox(CreateChatbox(mWorld), playerStateComponent.mRivalName.GetString() + ": Gramps!#I'm fed up with#waiting!", mWorld);
        mTimer.Reset();

        mEventState = EventState::GARY_COMPLAINING_CONVERSATION;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateGaryComplainingConversation(const float dt)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        mTimer.Update(dt);
        if (mTimer.HasTicked())
        {
            QueueDialogForChatbox
            (
                CreateChatbox(mWorld),
                "OAK: Hmm? " + playerStateComponent.mRivalName.GetString() + "?#Why are you here#already?#@I said for you to#come by later...#@" + 
                "Ah, whatever!#Just wait there.#@Look, " + playerStateComponent.mPlayerTrainerName.GetString() + "! Do#you see that ball#on the table?#@" + 
                "It's called a POK^#BALL. It holds a#POK^MON inside.#@You may have it!#Go on, take it!", 
                mWorld
            );
            mTimer.Reset();

            mEventState = EventState::OAK_URGING_PLAYER_TO_TAKE_BALL_CONVERSATION;
        }
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateOakUrgingPlayerToTakeBallConversation(const float dt)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        mTimer.Update(dt);
        if (mTimer.HasTicked())
        {            
            QueueDialogForChatbox
            (
                CreateChatbox(mWorld),
                playerStateComponent.mRivalName.GetString() + ": Hey!#Gramps! What#about me?",
                mWorld
            );
            
            mTimer.Reset();

            mEventState = EventState::GARY_COMPLAINING_AGAIN_CONVERSATION;
        }
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateGaryComplainingAgainConversation(const float dt)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        mTimer.Update(dt);
        if (mTimer.HasTicked())
        {
            QueueDialogForChatbox
            (
                CreateChatbox(mWorld),
                "OAK: Be patient,#" + playerStateComponent.mRivalName.GetString() + ", I'll give#you one later.",
                mWorld
            );

            mTimer.Reset();

            mEventState = EventState::OAK_RESPONDING_TO_GARY_COMPLAINTS_CONVERSATION;
        }
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateOakComplainingToGaryComplaintsConversation(const float dt)
{    
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        mTimer.Update(dt);
        if (mTimer.HasTicked())
        {            
            CompleteOverworldFlow();
        }
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::CreateExlamationMark()
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
    exclamationMarkTransformComponent->mPosition = mIsPlayerOnLeftTile ?
        TileCoordsToPosition(OAK_SPEECH_COORDS_1.mCol, OAK_SPEECH_COORDS_1.mRow + 1) :
        TileCoordsToPosition(OAK_SPEECH_COORDS_2.mCol, OAK_SPEECH_COORDS_2.mRow + 1);

    exclamationMarkTransformComponent->mPosition.y += GAME_TILE_SIZE;

    mWorld.AddComponent<RenderableComponent>(mExclamationMarkEntityId, std::move(exclamationMarkRenderableComponent));
    mWorld.AddComponent<TransformComponent>(mExclamationMarkEntityId, std::move(exclamationMarkTransformComponent));
}

void PikachuCatchIntroSequenceOverworldFlowState::PositionOakSprite(const TimelinePoint timelinePoint)
{    
    auto oakEntityId = GetNpcEntityIdFromLevelIndex(PALLET_OAK_LEVEL_INDEX, mWorld);
    if (timelinePoint == TimelinePoint::OAKS_LAB_ENTRY || timelinePoint == TimelinePoint::OAKS_LAB_FINAL)
    {
        oakEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_OAK_LEVEL_INDEX, mWorld);
    }

    auto& transformComponent     = mWorld.GetComponent<TransformComponent>(oakEntityId);
    auto& movementStateComponent = mWorld.GetComponent<MovementStateComponent>(oakEntityId);

    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    auto& levelModelComponent        = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));

    switch (timelinePoint)
    {
        case TimelinePoint::PALLET_TOWN_PRE_ENCOUNTER: 
        {
            transformComponent.mPosition = TileCoordsToPosition(OAK_ENTRANCE_COORDS);

            movementStateComponent.mCurrentCoords = OAK_ENTRANCE_COORDS;

            GetTile(OAK_ENTRANCE_COORDS, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = oakEntityId;
            GetTile(OAK_ENTRANCE_COORDS, levelModelComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::NPC;
        } break;

        case TimelinePoint::PALLET_TOWN_POST_ENCOUNTER: 
        {
            if (mIsPlayerOnLeftTile)
            {
                transformComponent.mPosition = TileCoordsToPosition(OAK_SPEECH_COORDS_1);

                movementStateComponent.mCurrentCoords = OAK_SPEECH_COORDS_1;

                GetTile(OAK_SPEECH_COORDS_1, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = oakEntityId;
                GetTile(OAK_SPEECH_COORDS_1, levelModelComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::NPC;

                ChangeCharacterDirection(Character::OAK, timelinePoint, Direction::EAST);
            }
            else
            {
                transformComponent.mPosition = TileCoordsToPosition(OAK_SPEECH_COORDS_2);

                movementStateComponent.mCurrentCoords = OAK_SPEECH_COORDS_2;

                GetTile(OAK_SPEECH_COORDS_2, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = oakEntityId;
                GetTile(OAK_SPEECH_COORDS_2, levelModelComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::NPC;

                ChangeCharacterDirection(Character::OAK, timelinePoint, Direction::WEST);
            }
        } break;

        case TimelinePoint::OAKS_LAB_ENTRY: 
        {
            const auto oaksLabEntryCoords = TileCoords(8, 3);
            transformComponent.mPosition = TileCoordsToPosition(oaksLabEntryCoords);

            movementStateComponent.mCurrentCoords = oaksLabEntryCoords;

            GetTile(oaksLabEntryCoords, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = oakEntityId;
            GetTile(oaksLabEntryCoords, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NPC;

            ChangeCharacterDirection(Character::OAK, timelinePoint, Direction::NORTH);
        } break;

        case TimelinePoint::OAKS_LAB_FINAL:
        {
            const auto oaksLabMidCoords = TileCoords(8, 8);

            GetTile(oaksLabMidCoords, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = ecs::NULL_ENTITY_ID;
            GetTile(oaksLabMidCoords, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NONE;

            const auto oaksLabFinalCoords = TileCoords(8, 11);
            transformComponent.mPosition = TileCoordsToPosition(oaksLabFinalCoords);

            movementStateComponent.mCurrentCoords = oaksLabFinalCoords;

            GetTile(oaksLabFinalCoords, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = oakEntityId;
            GetTile(oaksLabFinalCoords, levelModelComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::NPC;

            ChangeCharacterDirection(Character::OAK, timelinePoint, Direction::SOUTH);
        } break;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::CreateOakEntranceScriptedPath()
{
    const auto oakEntityId = GetNpcEntityIdFromLevelIndex(PALLET_OAK_LEVEL_INDEX, mWorld);
    auto& oakAiComponent = mWorld.GetComponent<NpcAiComponent>(oakEntityId);
    oakAiComponent.mAiTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);    

    if (mIsPlayerOnLeftTile)
    {
        oakAiComponent.mScriptedPathTileCoords.emplace_back(OAK_SPEECH_COORDS_1.mCol, OAK_SPEECH_COORDS_1.mRow);
    }
    else
    {
        oakAiComponent.mScriptedPathTileCoords.emplace_back(OAK_SPEECH_COORDS_2.mCol - 1, OAK_SPEECH_COORDS_2.mRow - 1);
        oakAiComponent.mScriptedPathTileCoords.emplace_back(OAK_SPEECH_COORDS_2.mCol, OAK_SPEECH_COORDS_2.mRow - 1);
        oakAiComponent.mScriptedPathTileCoords.emplace_back(OAK_SPEECH_COORDS_2.mCol, OAK_SPEECH_COORDS_2.mRow);
    }    

    oakAiComponent.mScriptedPathIndex = 0;
}

void PikachuCatchIntroSequenceOverworldFlowState::CreateScriptedPathToLab()
{
    const auto oakEntityId    = GetNpcEntityIdFromLevelIndex(PALLET_OAK_LEVEL_INDEX, mWorld);
    const auto playerEntityId = GetPlayerEntityId(mWorld);

    auto oakAiComponent      = &mWorld.GetComponent<NpcAiComponent>(oakEntityId);
    oakAiComponent->mAiTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);

    auto playerNpcAiComponent = std::make_unique<NpcAiComponent>();       

    if (!mIsPlayerOnLeftTile)
    {
        playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(17, 22);
        playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(16, 22);
        
        oakAiComponent->mScriptedPathTileCoords.emplace_back(16, 22);        
    }

    oakAiComponent->mScriptedPathTileCoords.emplace_back(16, 14);
    oakAiComponent->mScriptedPathTileCoords.emplace_back(15, 14);
    oakAiComponent->mScriptedPathTileCoords.emplace_back(15, 11);
    oakAiComponent->mScriptedPathTileCoords.emplace_back(18, 11);
    oakAiComponent->mScriptedPathTileCoords.emplace_back(18, 12);

    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(16, 14);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(15, 14);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(15, 11);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(18, 11);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(18, 12);

    playerNpcAiComponent->mScriptedPathIndex = 0;
    oakAiComponent->mScriptedPathIndex       = 0;

    mWorld.AddComponent<NpcAiComponent>(playerEntityId, std::move(playerNpcAiComponent));
}

void PikachuCatchIntroSequenceOverworldFlowState::CreateOakPathInLab()
{
    const auto oakEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_OAK_LEVEL_INDEX, mWorld);
    auto& oakAiComponent   = mWorld.GetComponent<NpcAiComponent>(oakEntityId);

    oakAiComponent.mScriptedPathTileCoords.emplace_back(8, 8);
    oakAiComponent.mScriptedPathIndex = 0;
}

void PikachuCatchIntroSequenceOverworldFlowState::CreatePlayerMovingToOakPathInLab()
{
    const auto playerEntityId = GetPlayerEntityId(mWorld);

    auto playerNpcAiComponent = std::make_unique<NpcAiComponent>();

    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(8, 10);
    playerNpcAiComponent->mScriptedPathIndex = 0;

    mWorld.AddComponent<NpcAiComponent>(playerEntityId, std::move(playerNpcAiComponent));
}

void PikachuCatchIntroSequenceOverworldFlowState::ChangeCharacterDirection(const Character character, const TimelinePoint timelinePoint, const Direction direction) const
{
    auto characterEntityId = GetPlayerEntityId(mWorld);

    if (character == Character::OAK)
    {
        characterEntityId = GetNpcEntityIdFromLevelIndex(PALLET_OAK_LEVEL_INDEX, mWorld);
        if (timelinePoint == TimelinePoint::OAKS_LAB_ENTRY || timelinePoint == TimelinePoint::OAKS_LAB_FINAL)
        {
            characterEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_OAK_LEVEL_INDEX, mWorld);
        }
    }
    else if (character == Character::GARY)
    {
        characterEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_LEVEL_INDEX, mWorld);
    }

    auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(characterEntityId);
    auto& directionComponent  = mWorld.GetComponent<DirectionComponent>(characterEntityId);

    directionComponent.mDirection = direction;
    ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(direction), renderableComponent);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
