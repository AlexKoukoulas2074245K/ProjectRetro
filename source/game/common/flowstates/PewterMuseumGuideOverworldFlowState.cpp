//
//  PewterMuseumGuideOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 05/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PewterMuseumGuideOverworldFlowState.h"
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

const TileCoords PewterMuseumGuideOverworldFlowState::NPC_INITIAL_COORDS = TileCoords(38, 34);

const std::string PewterMuseumGuideOverworldFlowState::LEVEL_MUSIC_NAME = "viridian";
const std::string PewterMuseumGuideOverworldFlowState::FOLLOW_MUSIC_NAME = "follow";

const glm::vec3 PewterMuseumGuideOverworldFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.065f, -0.4f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PewterMuseumGuideOverworldFlowState::PewterMuseumGuideOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mEventState(EventState::INTRO_DIALOG)
{
    CreateYesNoTextbox(mWorld, YES_NO_TEXTBOX_POSITION);
}

void PewterMuseumGuideOverworldFlowState::VUpdate(const float)
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

void PewterMuseumGuideOverworldFlowState::UpdateIntroDialog()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    // Yes/No textbox active
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
        const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
        const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
        const auto& npcComponent         = mWorld.GetComponent<NpcAiComponent>(GetNpcEntityIdFromLevelIndex(playerStateComponent.mLastNpcLevelIndexSpokenTo, mWorld));
        
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
                const auto mainChatbox = CreateChatbox(mWorld);
                QueueDialogForChatbox(mainChatbox, npcComponent.mSideDialogs[0], mWorld);
            }
            // No Selected
            else if (yesNoTextboxCursorRow == 1)
            {
                const auto mainChatbox = CreateChatbox(mWorld);
                QueueDialogForChatbox(mainChatbox, npcComponent.mSideDialogs[1], mWorld);
                
                mEventState = EventState::CONSTRUCT_PATH;
            }
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);
            
            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);
            
            const auto mainChatbox = CreateChatbox(mWorld);
            QueueDialogForChatbox(mainChatbox, npcComponent.mSideDialogs[1], mWorld);
            
            mEventState = EventState::CONSTRUCT_PATH;
        }
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        CompleteOverworldFlow();
    }
}

void PewterMuseumGuideOverworldFlowState::UpdateConstructPath()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        SoundService::GetInstance().PlayMusic(FOLLOW_MUSIC_NAME, false);
        CreateScriptedPath();
        mEventState = EventState::FOLLOWING;
    }
}

void PewterMuseumGuideOverworldFlowState::UpdateFollowingNpc()
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    const auto npcEntityId    = GetNpcEntityIdFromLevelIndex(playerStateComponent.mLastNpcLevelIndexSpokenTo, mWorld);
    
    auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    
    if (npcAiComponent.mScriptedPathIndex == -1)
    {
        auto& npcDirectionComponent      = mWorld.GetComponent<DirectionComponent>(npcEntityId);
        npcDirectionComponent.mDirection = Direction::NORTH;
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(npcDirectionComponent.mDirection), mWorld.GetComponent<RenderableComponent>(npcEntityId));
        
        mWorld.RemoveComponent<NpcAiComponent>(playerEntityId);
        
        SoundService::GetInstance().PlayMusic(LEVEL_MUSIC_NAME, false);
        
        QueueDialogForChatbox(CreateChatbox(mWorld), "It's right here!#You have to pay#to get in, but#it's worth it!#See you around!", mWorld);
        
        mEventState = EventState::END_DIALOG;
    }
}

void PewterMuseumGuideOverworldFlowState::UpdateEndDialog()
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        const auto npcEntityId = GetNpcEntityIdFromLevelIndex(playerStateComponent.mLastNpcLevelIndexSpokenTo, mWorld);
        auto& otherAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
        otherAiComponent.mScriptedPathTileCoords.emplace_back(23,37);
        otherAiComponent.mScriptedPathIndex = 0;
        mEventState = EventState::END_PATH;
    }
}

void PewterMuseumGuideOverworldFlowState::UpdateEndPath()
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto npcEntityId    = GetNpcEntityIdFromLevelIndex(playerStateComponent.mLastNpcLevelIndexSpokenTo, mWorld);
    
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
        
        GetTile(movementStateComponent.mCurrentCoords.mCol, movementStateComponent.mCurrentCoords.mRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = ecs::NULL_ENTITY_ID;
        GetTile(movementStateComponent.mCurrentCoords.mCol, movementStateComponent.mCurrentCoords.mRow, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NONE;
        
        movementStateComponent.mCurrentCoords = NPC_INITIAL_COORDS;
        
        GetTile(NPC_INITIAL_COORDS.mCol, NPC_INITIAL_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = npcEntityId;
        GetTile(NPC_INITIAL_COORDS.mCol, NPC_INITIAL_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NPC;
        
        CompleteOverworldFlow();
    }
}

void PewterMuseumGuideOverworldFlowState::CreateScriptedPath()
{
    // Construct path for player
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    
    const auto& playerStateComponent     = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& playerDirectionComponent = mWorld.GetComponent<DirectionComponent>(playerEntityId);
    
    auto playerNpcAiComponent = std::make_unique<NpcAiComponent>();
    
    if (playerDirectionComponent.mDirection == Direction::SOUTH)
    {
        playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(37,35);
        playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(38,35);
    }
    else
    {
        playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(38,34);
    }
    
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(38,40);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(24,40);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(24,43);
    playerNpcAiComponent->mScriptedPathIndex = 0;
    
    mWorld.AddComponent<NpcAiComponent>(playerEntityId, std::move(playerNpcAiComponent));
    
    // Construct path for npc
    const auto npcEntityId = GetNpcEntityIdFromLevelIndex(playerStateComponent.mLastNpcLevelIndexSpokenTo, mWorld);
    auto& otherAiComponent = mWorld.GetComponent<NpcAiComponent>(npcEntityId);
    otherAiComponent.mAiTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);
    otherAiComponent.mScriptedPathTileCoords.emplace_back(38,40);
    otherAiComponent.mScriptedPathTileCoords.emplace_back(24,40);
    otherAiComponent.mScriptedPathTileCoords.emplace_back(24,43);
    otherAiComponent.mScriptedPathTileCoords.emplace_back(23,43);
    otherAiComponent.mScriptedPathIndex = 0;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
