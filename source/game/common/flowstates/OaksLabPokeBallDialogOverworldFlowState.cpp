//
//  OaksLabPokeBallDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 23/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OaksLabPokeBallDialogOverworldFlowState.h"
#include "../components/DirectionComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/PokedexUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../overworld/utils/OverworldCharacterLoadingUtils.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/utils/AnimationUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const TileCoords OaksLabPokeBallDialogOverworldFlowState::EXCLAMATION_MARK_ATLAS_COORDS = TileCoords(7, 46);
const TileCoords OaksLabPokeBallDialogOverworldFlowState::OAKS_LAB_POKEBALL_COORDS      = TileCoords(10, 10);

const int OaksLabPokeBallDialogOverworldFlowState::OAKS_LAB_OAK_LEVEL_INDEX             = 10;
const int OaksLabPokeBallDialogOverworldFlowState::OAKS_LAB_GARY_LEVEL_INDEX            = 11;
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
            case EventState::EXCLAMATION_MARK:                    UpdateExclamationMark(dt); break;
            case EventState::GARY_MOVING_TOWARD_PLAYER:           UpdateGaryMovingTowardPlayer(); break;
            case EventState::PLAYER_MOVING_TOWARD_WALL:           UpdatePlayerMovingTowardWall(); break;
            case EventState::GARY_SNATCHING_POKEMON_CONVERSATION: UpdateGarySnatchingPokemonConversation(); break;
            case EventState::OAK_POST_SNATCH_CONVERSATION:        UpdateOakPostSnatchConversation(); break;
            case EventState::PLAYER_MOVING_TOWARD_OAK:            UpdatePlayerMovingTowardOak(); break;
            case EventState::PIKACHU_RECEPTION_CONVERSATION:      UpdatePikachuReceptionConversation(); break;
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
            CreateGaryMovingToPlayerPath();
            mEventState = EventState::GARY_MOVING_TOWARD_PLAYER;
        }
    }    
}

void OaksLabPokeBallDialogOverworldFlowState::UpdateGaryMovingTowardPlayer()
{
    const auto garyEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_LEVEL_INDEX, mWorld);
    auto& garyAiComponent   = mWorld.GetComponent<NpcAiComponent>(garyEntityId);
    if (garyAiComponent.mScriptedPathIndex == -1)
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
        mWorld.RemoveComponent<NpcAiComponent>(playerEntityId);
        mWorld.DestroyEntity(FindEntityAtLevelCoords(OAKS_LAB_POKEBALL_COORDS, mWorld));
        DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(OAKS_LAB_POKEBALL_ENTITY_LEVEL_INDEX, mWorld), mWorld);

        const auto garyEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_LEVEL_INDEX, mWorld);

        auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(garyEntityId);
        npcAiComponent.mInitDirection = Direction::NORTH;    

        auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(garyEntityId);
        auto& directionComponent  = mWorld.GetComponent<DirectionComponent>(garyEntityId);

        directionComponent.mDirection = Direction::NORTH;
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::NORTH), renderableComponent);

        QueueDialogForChatbox
        (
            CreateChatbox(mWorld),
            playerStateComponent.mRivalName.GetString() + ": No way!#" + playerStateComponent.mPlayerTrainerName.GetString() + ", I want#this POK^MON!#@" + 
            playerStateComponent.mRivalName.GetString() + " snatched#the POK^MON!",
            mWorld
        );

        mEventState = EventState::GARY_SNATCHING_POKEMON_CONVERSATION;
    }

}

void OaksLabPokeBallDialogOverworldFlowState::UpdateGarySnatchingPokemonConversation()
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
            
            // TODO: nickname flow
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

        auto& garyNpcAiComponent   = mWorld.GetComponent<NpcAiComponent>(GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_LEVEL_INDEX, mWorld));        
        garyNpcAiComponent.mDialog = playerStateComponent.mPlayerTrainerName.GetString() + ": Heh, my#POK^MON looks a#lot stronger.";

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
    const auto& garyMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_LEVEL_INDEX, mWorld));
    auto exclamationMarkTransformComponent = std::make_unique<TransformComponent>();
    exclamationMarkTransformComponent->mPosition = TileCoordsToPosition(garyMovementStateComponent.mCurrentCoords);
    exclamationMarkTransformComponent->mPosition.y += GAME_TILE_SIZE;

    mWorld.AddComponent<RenderableComponent>(mExclamationMarkEntityId, std::move(exclamationMarkRenderableComponent));
    mWorld.AddComponent<TransformComponent>(mExclamationMarkEntityId, std::move(exclamationMarkTransformComponent));
}

void OaksLabPokeBallDialogOverworldFlowState::CreateGaryMovingToPlayerPath()
{
    const auto garyEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_LEVEL_INDEX, mWorld);

    auto& garyAiComponent    = mWorld.GetComponent<NpcAiComponent>(garyEntityId);
    garyAiComponent.mAiTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);
    garyAiComponent.mScriptedPathTileCoords.emplace_back(7, 9);
    garyAiComponent.mScriptedPathTileCoords.emplace_back(9, 9);
    garyAiComponent.mScriptedPathIndex = 0;
}

void OaksLabPokeBallDialogOverworldFlowState::CreatePlayerMovingToWallPath()
{
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    auto playerNpcAiComponent = std::make_unique<NpcAiComponent>();

    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(12, 9);    
    playerNpcAiComponent->mScriptedPathIndex = 0;    

    const auto garyEntityId = GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_LEVEL_INDEX, mWorld);
    auto& garyAiComponent = mWorld.GetComponent<NpcAiComponent>(garyEntityId);
    garyAiComponent.mScriptedPathTileCoords.emplace_back(10, 9);
    garyAiComponent.mScriptedPathIndex = 0;
    
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
