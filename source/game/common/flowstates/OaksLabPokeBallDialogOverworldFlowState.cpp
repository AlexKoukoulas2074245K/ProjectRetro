//
//  OaksLabPokeBallDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 23/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "NameSelectionFlowState.h"
#include "OaksLabPokeBallDialogOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/DirectionComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/NameSelectionStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/PokedexUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/LevelModelComponent.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../overworld/utils/OverworldCharacterLoadingUtils.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/utils/AnimationUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 OaksLabPokeBallDialogOverworldFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.065f, -0.4f);

const TileCoords OaksLabPokeBallDialogOverworldFlowState::EXCLAMATION_MARK_ATLAS_COORDS = TileCoords(7, 46);
const TileCoords OaksLabPokeBallDialogOverworldFlowState::OAKS_LAB_POKEBALL_COORDS      = TileCoords(10, 10);

const int OaksLabPokeBallDialogOverworldFlowState::OAKS_LAB_OAK_LEVEL_INDEX             = 10;
const int OaksLabPokeBallDialogOverworldFlowState::OAKS_LAB_RIVAL_LEVEL_INDEX            = 11;
const int OaksLabPokeBallDialogOverworldFlowState::OAKS_LAB_POKEBALL_ENTITY_LEVEL_INDEX = 12;

const float OaksLabPokeBallDialogOverworldFlowState::EXCLAMATION_MARK_LIFE_TIME         = 1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OaksLabPokeBallDialogOverworldFlowState::OaksLabPokeBallDialogOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mExclamationMarkEntityId(ecs::NULL_ENTITY_ID)
    , mTimer(EXCLAMATION_MARK_LIFE_TIME)
    , mEventState(EventState::EXCLAMATION_MARK)
{
    DestroyActiveTextbox(mWorld);
    
    if (!HasMilestone(milestones::SEEN_OAK_FIRST_TIME, mWorld))
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "That's a POK^#BALL. There's a#POK^MON inside!", mWorld);
    }    
}

void OaksLabPokeBallDialogOverworldFlowState::VUpdate(const float dt)
{    
    if (!HasMilestone(milestones::SEEN_OAK_FIRST_TIME, mWorld))
    {
        if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
        {
            CompleteOverworldFlow();
        }
    }
    else
    {
        switch (mEventState)
        {
            case EventState::EXCLAMATION_MARK:                     UpdateExclamationMark(dt); break;
            case EventState::RIVAL_MOVING_TOWARD_PLAYER:           UpdateRivalMovingTowardPlayer(); break;
            case EventState::PLAYER_MOVING_TOWARD_WALL:            UpdatePlayerMovingTowardWall(); break;
            case EventState::RIVAL_SNATCHING_POKEMON_CONVERSATION: UpdateRivalSnatchingPokemonConversation(); break;
            case EventState::OAK_POST_SNATCH_CONVERSATION:         UpdateOakPostSnatchConversation(); break;
            case EventState::PLAYER_MOVING_TOWARD_OAK:             UpdatePlayerMovingTowardOak(); break;
            case EventState::PIKACHU_RECEPTION_CONVERSATION:       UpdatePikachuReceptionConversation(); break;
            case EventState::PIKACHU_NICKNAME_FLOW:                UpdatePikachuNicknameFlow(); break;
            case EventState::PIKACHU_NICKNAME_YES_NO:              UpdatePikachuNicknameYesNoFlow(); break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void OaksLabPokeBallDialogOverworldFlowState::UpdateExclamationMark(const float dt)
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
            CreateRivalMovingToPlayerPath();
            mEventState = EventState::RIVAL_MOVING_TOWARD_PLAYER;
        }
    }    
}

void OaksLabPokeBallDialogOverworldFlowState::UpdateRivalMovingTowardPlayer()
{
    const auto rivalEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_RIVAL_LEVEL_INDEX, mWorld);
    auto& rivalAiComponent   = mWorld.GetComponent<NpcAiComponent>(rivalEntityId);
    if (rivalAiComponent.mScriptedPathIndex == -1)
    {
        CreatePlayerMovingToWallPath();
        mEventState = EventState::PLAYER_MOVING_TOWARD_WALL;
    }
}

void OaksLabPokeBallDialogOverworldFlowState::UpdatePlayerMovingTowardWall()
{
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& playerNpcAiComponent = mWorld.GetComponent<NpcAiComponent>(playerEntityId);

    if (playerNpcAiComponent.mScriptedPathIndex == -1)
    {
        const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
        auto& levelModelComponent = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));

        mWorld.RemoveComponent<NpcAiComponent>(playerEntityId);                  

        const auto rivalEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_RIVAL_LEVEL_INDEX, mWorld);

        auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(rivalEntityId);
        npcAiComponent.mInitDirection = Direction::NORTH;    

        auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(rivalEntityId);
        auto& directionComponent  = mWorld.GetComponent<DirectionComponent>(rivalEntityId);

        directionComponent.mDirection = Direction::NORTH;
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::NORTH), renderableComponent);

        QueueDialogForChatbox
        (
            CreateChatbox(mWorld),
            playerStateComponent.mRivalName.GetString() + ": No way!#" + playerStateComponent.mPlayerTrainerName.GetString() + ", I want#this POK^MON!#@" + 
            playerStateComponent.mRivalName.GetString() + " snatched#the POK^MON!",
            mWorld
        );

        DestroyOverworldModelNpcAndEraseTileInfo(OAKS_LAB_POKEBALL_COORDS, mWorld);

        GetTile(10, 9, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = rivalEntityId;
        GetTile(10, 9, levelModelComponent.mLevelTilemap).mTileOccupierType = TileOccupierType::NPC;
        
        mEventState = EventState::RIVAL_SNATCHING_POKEMON_CONVERSATION;
    }

}

void OaksLabPokeBallDialogOverworldFlowState::UpdateRivalSnatchingPokemonConversation()
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        QueueDialogForChatbox
        (
            CreateChatbox(mWorld),
            "OAK: " + playerStateComponent.mRivalName.GetString() + "! What#are you doing?#@" + 
            playerStateComponent.mRivalName.GetString() + ": Gramps, I#want this one!#@" + 
            "OAK: But, I... Oh,#all right then.#That POK^MON is#yours.#@I was going to#give you one#anyway...#@" + 
            playerStateComponent.mPlayerTrainerName.GetString() + ", come over#here.",
            mWorld
        );

        mEventState = EventState::OAK_POST_SNATCH_CONVERSATION;
    }
}


void OaksLabPokeBallDialogOverworldFlowState::UpdateOakPostSnatchConversation()
{
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        CreatePlayerMovingToOakPath();
        mEventState = EventState::PLAYER_MOVING_TOWARD_OAK;
    }
}

void OaksLabPokeBallDialogOverworldFlowState::UpdatePlayerMovingTowardOak()
{
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& playerNpcAiComponent = mWorld.GetComponent<NpcAiComponent>(playerEntityId);

    if (playerNpcAiComponent.mScriptedPathIndex == -1)
    {
        mWorld.RemoveComponent<NpcAiComponent>(playerEntityId);

        QueueDialogForChatbox
        (
            CreateChatbox(mWorld),
            "OAK: " + playerStateComponent.mPlayerTrainerName.GetString() + ", this#is the POK^MON I#" +
            "caught earlier.#@You can have it.#I caught it in#the wild and it's#not tame yet.#@" +
            playerStateComponent.mPlayerTrainerName.GetString() + " received#a PIKACHU!",
            mWorld
        );

        mEventState = EventState::PIKACHU_RECEPTION_CONVERSATION;
    }
}

void OaksLabPokeBallDialogOverworldFlowState::UpdatePikachuReceptionConversation()
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        SetMilestone(milestones::RECEIVED_PIKACHU, mWorld);

        auto& rivalNpcAiComponent   = mWorld.GetComponent<NpcAiComponent>(GetNpcEntityIdFromLevelIndex(OAKS_LAB_RIVAL_LEVEL_INDEX, mWorld));        
        rivalNpcAiComponent.mDialog = playerStateComponent.mPlayerTrainerName.GetString() + ": Heh, my#POK^MON looks a#lot stronger.";

        QueueDialogForChatbox(CreateChatbox(mWorld), "Do you want to#give a nickname#to PIKACHU?+FREEZE", mWorld);
        
        mEventState = EventState::PIKACHU_NICKNAME_FLOW;
    }
}

void OaksLabPokeBallDialogOverworldFlowState::UpdatePikachuNicknameFlow()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        CreateYesNoTextbox(mWorld, YES_NO_TEXTBOX_POSITION);
        mEventState = EventState::PIKACHU_NICKNAME_YES_NO;
    }
}

void OaksLabPokeBallDialogOverworldFlowState::UpdatePikachuNicknameYesNoFlow()
{
    const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& nameSelectionComponent     = mWorld.GetSingletonComponent<NameSelectionStateSingletonComponent>();
    
    const auto yesNoTextboxCursorRow = cursorComponent.mCursorRow;
    
    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        // Destroy Yes/No textbox
        DestroyActiveTextbox(mWorld);
        
        // Destroy main chatbox
        DestroyActiveTextbox(mWorld);
        
        // Yes Selected
        if (yesNoTextboxCursorRow == 0)
        {
            nameSelectionComponent.mNameSelectionMode = NameSelectionMode::POKEMON_NICKNAME;
            nameSelectionComponent.mPokemonToSelectNameFor = playerStateComponent.mPlayerPokemonRoster.back().get();
            CompleteAndTransitionTo<NameSelectionFlowState>();
        }
        // No Selected
        else if (yesNoTextboxCursorRow == 1)
        {
            CompleteOverworldFlow();
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        // Destroy Yes/No textbox
        DestroyActiveTextbox(mWorld);
        
        // Destroy main chatbox
        DestroyActiveTextbox(mWorld);
        
        CompleteOverworldFlow();
    }
}

void OaksLabPokeBallDialogOverworldFlowState::CreateExlamationMark()
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
    const auto& rivalMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(GetNpcEntityIdFromLevelIndex(OAKS_LAB_RIVAL_LEVEL_INDEX, mWorld));
    auto exclamationMarkTransformComponent = std::make_unique<TransformComponent>();
    exclamationMarkTransformComponent->mPosition = TileCoordsToPosition(rivalMovementStateComponent.mCurrentCoords);
    exclamationMarkTransformComponent->mPosition.y += GAME_TILE_SIZE;

    mWorld.AddComponent<RenderableComponent>(mExclamationMarkEntityId, std::move(exclamationMarkRenderableComponent));
    mWorld.AddComponent<TransformComponent>(mExclamationMarkEntityId, std::move(exclamationMarkTransformComponent));
}

void OaksLabPokeBallDialogOverworldFlowState::CreateRivalMovingToPlayerPath()
{
    const auto rivalEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_RIVAL_LEVEL_INDEX, mWorld);

    auto& rivalAiComponent    = mWorld.GetComponent<NpcAiComponent>(rivalEntityId);
    rivalAiComponent.mAiTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);
    rivalAiComponent.mScriptedPathTileCoords.emplace_back(7, 9);
    rivalAiComponent.mScriptedPathTileCoords.emplace_back(9, 9);
    rivalAiComponent.mScriptedPathIndex = 0;
}

void OaksLabPokeBallDialogOverworldFlowState::CreatePlayerMovingToWallPath()
{
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    auto playerNpcAiComponent = std::make_unique<NpcAiComponent>();

    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(12, 9);    
    playerNpcAiComponent->mScriptedPathIndex = 0;    

    const auto rivalEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_RIVAL_LEVEL_INDEX, mWorld);
    auto& rivalAiComponent = mWorld.GetComponent<NpcAiComponent>(rivalEntityId);
    rivalAiComponent.mScriptedPathTileCoords.emplace_back(10, 9);
    rivalAiComponent.mScriptedPathIndex = 0;
    
    mWorld.AddComponent<NpcAiComponent>(playerEntityId, std::move(playerNpcAiComponent));
}

void OaksLabPokeBallDialogOverworldFlowState::CreatePlayerMovingToOakPath()
{
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    auto playerNpcAiComponent = std::make_unique<NpcAiComponent>();

    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(11, 9);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(11, 8);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(8, 8);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(8, 10);
    playerNpcAiComponent->mScriptedPathIndex = 0;

    mWorld.AddComponent<NpcAiComponent>(playerEntityId, std::move(playerNpcAiComponent));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
