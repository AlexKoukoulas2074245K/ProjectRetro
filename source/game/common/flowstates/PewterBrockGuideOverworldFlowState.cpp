//
//  PewterBrockGuideOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 06/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PewterBrockGuideOverworldFlowState.h"
#include "../components/DirectionComponent.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/components/NpcAiComponent.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const TileCoords PewterBrockGuideOverworldFlowState::NPC_INITIAL_COORDS = TileCoords(46, 35);

const std::string PewterBrockGuideOverworldFlowState::LEVEL_MUSIC_NAME = "viridian";
const std::string PewterBrockGuideOverworldFlowState::FOLLOW_MUSIC_NAME = "follow";

const int PewterBrockGuideOverworldFlowState::NPC_LEVEL_INDEX = 2;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PewterBrockGuideOverworldFlowState::PewterBrockGuideOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mEventState(EventState::INTRO_DIALOG)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        DestroyActiveTextbox(mWorld);
    }
    
    QueueDialogForChatbox(CreateChatbox(mWorld), "You're a trainer#right? BROCK's#looking for new#challengers!#Follow me!", mWorld);
}

void PewterBrockGuideOverworldFlowState::VUpdate(const float)
{
    switch (mEventState)
    {
        case EventState::INTRO_DIALOG:   UpdateIntroDialog(); break;
        case EventState::CONSTRUCT_PATH: UpdateConstructPath(); break;
        case EventState::FOLLOWING:      UpdateFollowingNpc(); break;
        case EventState::END_DIALOG:     UpdateEndDialog(); break;
        case EventState::END_PATH:       UpdateEndPath(); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PewterBrockGuideOverworldFlowState::UpdateIntroDialog()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    // Intro dialog dismissed
    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        mEventState = EventState::CONSTRUCT_PATH;
    }
}

void PewterBrockGuideOverworldFlowState::UpdateConstructPath()
{
    SoundService::GetInstance().PlayMusic(FOLLOW_MUSIC_NAME, false);
    CreateScriptedPath();
    mEventState = EventState::FOLLOWING;
}

void PewterBrockGuideOverworldFlowState::UpdateFollowingNpc()
{
    
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    const auto npcEntityId    = GetNpcEntityIdFromLevelIndex(NPC_LEVEL_INDEX, mWorld);
    
    auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    
    if (npcAiComponent.mScriptedPathIndex == -1)
    {
        auto& npcDirectionComponent      = mWorld.GetComponent<DirectionComponent>(npcEntityId);
        npcDirectionComponent.mDirection = Direction::WEST;
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(npcDirectionComponent.mDirection), mWorld.GetComponent<RenderableComponent>(npcEntityId));
        
        mWorld.RemoveComponent<NpcAiComponent>(playerEntityId);
        
        SoundService::GetInstance().PlayMusic(LEVEL_MUSIC_NAME, false);
        
        QueueDialogForChatbox(CreateChatbox(mWorld), "If you have the#right stuff, go#take on BROCK!", mWorld);
        
        mEventState = EventState::END_DIALOG;
    }
}

void PewterBrockGuideOverworldFlowState::UpdateEndDialog()
{
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        const auto npcEntityId = GetNpcEntityIdFromLevelIndex(NPC_LEVEL_INDEX, mWorld);
        auto& otherAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
        otherAiComponent.mScriptedPathTileCoords.emplace_back(28,33);
        otherAiComponent.mScriptedPathIndex = 0;
        mEventState = EventState::END_PATH;
    }
}

void PewterBrockGuideOverworldFlowState::UpdateEndPath()
{
    const auto npcEntityId = GetNpcEntityIdFromLevelIndex(NPC_LEVEL_INDEX, mWorld);
    
    auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    
    if (npcAiComponent.mScriptedPathIndex == -1)
    {
        auto& npcDirectionComponent      = mWorld.GetComponent<DirectionComponent>(npcEntityId);
        npcDirectionComponent.mDirection = Direction::SOUTH;
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(npcDirectionComponent.mDirection), mWorld.GetComponent<RenderableComponent>(npcEntityId));
        
        npcAiComponent.mAiTimer = std::make_unique<Timer>(STATIONARY_NPC_RESET_TIME);
        
        const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
        auto& levelModelComponent        = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
        
        auto& transformComponent       = mWorld.GetComponent<TransformComponent>(npcEntityId);
        transformComponent.mPosition = TileCoordsToPosition(NPC_INITIAL_COORDS.mCol, NPC_INITIAL_COORDS.mRow);
        
        auto& movementStateComponent = mWorld.GetComponent<MovementStateComponent>(npcEntityId);
        movementStateComponent.mCurrentCoords = NPC_INITIAL_COORDS;
        
        GetTile(movementStateComponent.mCurrentCoords.mCol, movementStateComponent.mCurrentCoords.mRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = npcEntityId;
        GetTile(movementStateComponent.mCurrentCoords.mCol, movementStateComponent.mCurrentCoords.mRow, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NPC;
        
        GetTile(NPC_INITIAL_COORDS.mCol, NPC_INITIAL_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = npcEntityId;
        GetTile(NPC_INITIAL_COORDS.mCol, NPC_INITIAL_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NPC;
        
        CompleteOverworldFlow();
    }
}

void PewterBrockGuideOverworldFlowState::CreateScriptedPath()
{
    // Construct path for player
    const auto playerEntityId = GetPlayerEntityId(mWorld);

    const auto& playerMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(playerEntityId);
    
    auto playerNpcAiComponent = std::make_unique<NpcAiComponent>();
    
    if (playerMovementStateComponent.mCurrentCoords == TileCoords(46,34))
    {
        playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(47,34);
        playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(46,34);
    }
    else if (playerMovementStateComponent.mCurrentCoords == TileCoords(47,33))
    {
        playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(47,34);
        playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(46,34);
    }
    else if (playerMovementStateComponent.mCurrentCoords == TileCoords(48,32))
    {
        playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(48,33);
    }
    else
    {
        playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(46,35);
    }

    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(46,33);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(31,33);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(31,38);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(20,38);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(20,33);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(21,33);
    
    playerNpcAiComponent->mScriptedPathIndex = 0;
    
    mWorld.AddComponent<NpcAiComponent>(playerEntityId, std::move(playerNpcAiComponent));
    
    // Construct path for npc
    const auto npcEntityId = GetNpcEntityIdFromLevelIndex(NPC_LEVEL_INDEX, mWorld);
    auto& otherAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    otherAiComponent.mAiTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);
    otherAiComponent.mScriptedPathTileCoords.emplace_back(46,33);
    otherAiComponent.mScriptedPathTileCoords.emplace_back(31,33);
    otherAiComponent.mScriptedPathTileCoords.emplace_back(31,38);
    otherAiComponent.mScriptedPathTileCoords.emplace_back(20,38);
    otherAiComponent.mScriptedPathTileCoords.emplace_back(20,33);
    otherAiComponent.mScriptedPathTileCoords.emplace_back(22,33);
    
    otherAiComponent.mScriptedPathIndex = 0;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
