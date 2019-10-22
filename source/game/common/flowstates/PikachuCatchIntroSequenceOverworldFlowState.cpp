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
const std::string PikachuCatchIntroSequenceOverworldFlowState::FOLLOW_MUSIC_NAME      = "follow";
const std::string PikachuCatchIntroSequenceOverworldFlowState::WILD_BATTLE_MUSIC_NAME = "wild_battle";

const StringId PikachuCatchIntroSequenceOverworldFlowState::PIKACHU_NAME = StringId("PIKACHU");

const TileCoords PikachuCatchIntroSequenceOverworldFlowState::EXCLAMATION_MARK_ATLAS_COORDS = TileCoords(7, 46);
const TileCoords PikachuCatchIntroSequenceOverworldFlowState::OAK_ENTRANCE_COORDS           = TileCoords(16, 18);
const TileCoords PikachuCatchIntroSequenceOverworldFlowState::OAK_SPEECH_COORDS_1           = TileCoords(16, 22);
const TileCoords PikachuCatchIntroSequenceOverworldFlowState::OAK_SPEECH_COORDS_2           = TileCoords(17, 22);

const int PikachuCatchIntroSequenceOverworldFlowState::OAK_LEVEL_INDEX = 6;
const int PikachuCatchIntroSequenceOverworldFlowState::PIKACHU_LEVEL   = 5;

const float PikachuCatchIntroSequenceOverworldFlowState::EXCLAMATION_MARK_LIFE_TIME = 1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PikachuCatchIntroSequenceOverworldFlowState::PikachuCatchIntroSequenceOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mExclamationMarkTimer(EXCLAMATION_MARK_LIFE_TIME)
    , mExclamationMarkEntityId(ecs::NULL_ENTITY_ID)    
    , mEventState(EventState::EXCLAMATION_MARK)
{
    const auto& playerMovementState = mWorld.GetComponent<MovementStateComponent>(GetPlayerEntityId(mWorld));
    mIsPlayerOnLeftTile = playerMovementState.mCurrentCoords == TileCoords(OAK_SPEECH_COORDS_1.mCol, OAK_SPEECH_COORDS_1.mRow + 1);
    SoundService::GetInstance().PlayMusic(OAK_APPEARS_MUSIC_NAME, false);
    QueueDialogForChatbox(CreateChatbox(mWorld), "OAK: Hey! Wait!#Don't go out!+FREEZE", mWorld);
}

void PikachuCatchIntroSequenceOverworldFlowState::VUpdate(const float dt)
{    
    switch (mEventState)
    {
        case EventState::EXCLAMATION_MARK:          UpdateExclamationMark(dt); break;
        case EventState::OAK_ENTRANCE:              UpdateOakEntrance(); break;
        case EventState::OAK_THAT_WAS_CLOSE_DIALOG: UpdateOakThatWasCloseDialog(); break;        
        case EventState::WAIT_FOR_PIKACHU_CAPTURE:  UpdateWaitForPikachuCapture(); break;
        case EventState::OAK_PHEW_DIALOG:           UpdateOakPhewDialog(); break;
        case EventState::OAK_TALL_GRASS_DIALOG:     UpdateOakTallGrassDialog(); break;
        case EventState::FOLLOWING_OAK_TO_LAB:      UpdateFollowingOakToLab(); break;
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
            mExclamationMarkTimer.Update(dt);
            if (mExclamationMarkTimer.HasTicked())
            {
                mWorld.DestroyEntity(mExclamationMarkEntityId);         
                DestroyActiveTextbox(mWorld);
                ChangeCharacterDirection(Character::PLAYER, Direction::SOUTH);
                PositionOakSprite(false);
                CreateOakEntranceScriptedPath();

                mEventState = EventState::OAK_ENTRANCE;
            }
        }        
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateOakEntrance()
{    
    const auto oakEntityId = GetNpcEntityIdFromLevelIndex(OAK_LEVEL_INDEX, mWorld);

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
            ChangeCharacterDirection(Character::OAK, Direction::EAST);
        }
        else
        {
            ChangeCharacterDirection(Character::OAK, Direction::WEST);
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
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    if (encounterStateComponent.mActiveEncounterType == EncounterType::NONE)
    {
        PositionOakSprite(true);
        QueueDialogForChatbox(CreateChatbox(mWorld), "OAK: Whew..", mWorld);
        mEventState = EventState::OAK_PHEW_DIALOG;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateOakPhewDialog()
{    
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        ChangeCharacterDirection(Character::OAK, Direction::NORTH);
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
        mEventState = EventState::FOLLOWING_OAK_TO_LAB;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::UpdateFollowingOakToLab()
{

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

void PikachuCatchIntroSequenceOverworldFlowState::PositionOakSprite(const bool postBattle)
{    
    const auto oakEntityId = GetNpcEntityIdFromLevelIndex(OAK_LEVEL_INDEX, mWorld);

    auto& transformComponent     = mWorld.GetComponent<TransformComponent>(oakEntityId);
    auto& movementStateComponent = mWorld.GetComponent<MovementStateComponent>(oakEntityId);

    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    auto& levelModelComponent        = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));

    if (postBattle)
    {
        if (mIsPlayerOnLeftTile)
        {
            transformComponent.mPosition = TileCoordsToPosition(OAK_SPEECH_COORDS_1.mCol, OAK_SPEECH_COORDS_1.mRow);

            movementStateComponent.mCurrentCoords = OAK_SPEECH_COORDS_1;

            GetTile(OAK_SPEECH_COORDS_1.mCol, OAK_SPEECH_COORDS_1.mRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = oakEntityId;
            GetTile(OAK_SPEECH_COORDS_1.mCol, OAK_SPEECH_COORDS_1.mRow, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NPC;

            ChangeCharacterDirection(Character::OAK, Direction::EAST);
        }
        else
        {
            transformComponent.mPosition = TileCoordsToPosition(OAK_SPEECH_COORDS_2.mCol, OAK_SPEECH_COORDS_2.mRow);

            movementStateComponent.mCurrentCoords = OAK_SPEECH_COORDS_2;

            GetTile(OAK_SPEECH_COORDS_2.mCol, OAK_SPEECH_COORDS_2.mRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = oakEntityId;
            GetTile(OAK_SPEECH_COORDS_2.mCol, OAK_SPEECH_COORDS_2.mRow, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NPC;

            ChangeCharacterDirection(Character::OAK, Direction::WEST);
        }        
    }
    else
    {
        transformComponent.mPosition = TileCoordsToPosition(OAK_ENTRANCE_COORDS.mCol, OAK_ENTRANCE_COORDS.mRow);

        movementStateComponent.mCurrentCoords = OAK_ENTRANCE_COORDS;

        GetTile(OAK_ENTRANCE_COORDS.mCol, OAK_ENTRANCE_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = oakEntityId;
        GetTile(OAK_ENTRANCE_COORDS.mCol, OAK_ENTRANCE_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::NPC;
    }
}

void PikachuCatchIntroSequenceOverworldFlowState::CreateOakEntranceScriptedPath()
{
    const auto oakEntityId = GetNpcEntityIdFromLevelIndex(OAK_LEVEL_INDEX, mWorld);
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

void PikachuCatchIntroSequenceOverworldFlowState::ChangeCharacterDirection(const Character character, const Direction direction) const
{
    auto characterEntityId = GetPlayerEntityId(mWorld);

    if (character == Character::OAK)
    {
        characterEntityId = GetNpcEntityIdFromLevelIndex(OAK_LEVEL_INDEX, mWorld);
    }

    auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(characterEntityId);
    auto& directionComponent  = mWorld.GetComponent<DirectionComponent>(characterEntityId);

    directionComponent.mDirection = direction;
    ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(direction), renderableComponent);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
