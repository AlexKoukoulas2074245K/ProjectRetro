//
//  ViridianOaksParcelOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 13/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ViridianOaksParcelOverworldFlowState.h"
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
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const int ViridianOaksParcelOverworldFlowState::FLOW_TRIGGER_TILE_COL = 7;
const int ViridianOaksParcelOverworldFlowState::FLOW_TRIGGER_TILE_ROW = 4;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ViridianOaksParcelOverworldFlowState::ViridianOaksParcelOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mEventState(EventState::INTRO_DIALOG)
{
    if (HasMilestone(milestones::RECEIVED_OAKS_PARCEL, mWorld))
    {
        mEventState = EventState::END_DIALOG;
    }
    else
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "Hey! You came from#PALLET TOWN?", mWorld);
    }
}

void ViridianOaksParcelOverworldFlowState::VUpdate(const float)
{
    switch (mEventState)
    {
        case EventState::INTRO_DIALOG:      UpdateIntroDialog(); break;
        case EventState::MOVING_TO_COUNTER: UpdateMovingToCounter(); break;
        case EventState::END_DIALOG:        UpdateEndDialog(); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void ViridianOaksParcelOverworldFlowState::UpdateIntroDialog()
{    
    // Yes/No textbox active
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        CreateScriptedPath();
        mEventState = EventState::MOVING_TO_COUNTER;
    }
}

void ViridianOaksParcelOverworldFlowState::UpdateMovingToCounter()
{
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(playerEntityId);
    
    if (npcAiComponent.mScriptedPathIndex == -1)
    {
        mWorld.RemoveComponent<NpcAiComponent>(playerEntityId);
     
        const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
        QueueDialogForChatbox(CreateChatbox(mWorld), "You know PROF.#OAK, right?#@His order came in.#Will you take it#to him?#@" + playerStateComponent.mPlayerTrainerName.GetString() + " got#OAK's_PARCEL!", mWorld);
        
        mEventState = EventState::END_DIALOG;
    }
}

void ViridianOaksParcelOverworldFlowState::UpdateEndDialog()
{    
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
        auto& levelModelComponent        = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
        
        GetTile(FLOW_TRIGGER_TILE_COL, FLOW_TRIGGER_TILE_ROW, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::PRESS_WARP;
        CompleteOverworldFlow();
    }
}

void ViridianOaksParcelOverworldFlowState::CreateScriptedPath()
{
    // Construct path for player
    const auto playerEntityId = GetPlayerEntityId(mWorld);
    
    auto playerNpcAiComponent = std::make_unique<NpcAiComponent>();
    
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(7,6);
    playerNpcAiComponent->mScriptedPathTileCoords.emplace_back(6,6);
    playerNpcAiComponent->mScriptedPathIndex = 0;
    
    mWorld.AddComponent<NpcAiComponent>(playerEntityId, std::move(playerNpcAiComponent));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
