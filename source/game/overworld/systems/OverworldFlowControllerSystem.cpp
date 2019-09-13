//
//  OverworldFlowControllerSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 08/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OverworldFlowControllerSystem.h"
#include "../components/ActiveLevelSingletonComponent.h"
#include "../components/LevelModelComponent.h"
#include "../components/MovementStateComponent.h"
#include "../components/OverworldFlowStateSingletonComponent.h"
#include "../utils/LevelUtils.h"
#include "../utils/OverworldUtils.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/components/PokedexStateSingletonComponent.h"
#include "../../common/flowstates/PokeCenterHealingIntroDialogFlowState.h"
#include "../../common/flowstates/ViridianCaterpieWeedleGuyOverworldFlowState.h"
#include "../../common/flowstates/ViridianGymLockedOverworldFlowState.h"
#include "../../common/flowstates/ViridianRudeGuyOverworldFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OverworldFlowControllerSystem::OverworldFlowControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    InitializeOverworldFlowState();
}

void OverworldFlowControllerSystem::VUpdateAssociatedComponents(const float dt) const
{
    auto& overworldFlowStateComponent = mWorld.GetSingletonComponent<OverworldFlowStateSingletonComponent>();
    
    if (overworldFlowStateComponent.mFlowHookTriggered)
    {
        overworldFlowStateComponent.mFlowHookTriggered = false;
        DetermineWhichFlowToStart();
    }
    
    if (overworldFlowStateComponent.mFlowHasJustFinished)
    {
        overworldFlowStateComponent.mFlowHasJustFinished = false;
        overworldFlowStateComponent.mFlowStateManager.SetActiveFlowState(nullptr);
    }
    else
    {
        overworldFlowStateComponent.mFlowStateManager.Update(dt);
    }     
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void OverworldFlowControllerSystem::InitializeOverworldFlowState() const
{
    mWorld.SetSingletonComponent<OverworldFlowStateSingletonComponent>(std::make_unique<OverworldFlowStateSingletonComponent>());
}

void OverworldFlowControllerSystem::DetermineWhichFlowToStart() const
{
    const auto& activeLevelComponent      = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& playerStateComponent      = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& pokedexStateComponent     = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
    const auto& activeLevelModelComponent = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
    const auto& playerMovementComponent   = mWorld.GetComponent<MovementStateComponent>(GetPlayerEntityId(mWorld));
    const auto& currentPlayerTileCoords   = playerMovementComponent.mCurrentCoords;
    const auto& currentPlayerTile         = GetTile(playerMovementComponent.mCurrentCoords, activeLevelModelComponent.mLevelTilemap);
    const auto lastNpcSpokenToLevelIndex  = playerStateComponent.mLastNpcLevelIndexSpokenTo;
    const auto flowStartedByTileTrigger   = currentPlayerTile.mTileTrait == TileTrait::FLOW_TRIGGER;

    if (activeLevelComponent.mActiveLevelNameId == StringId("viridian"))
    { 
        if (flowStartedByTileTrigger)
        {
            // Rude guy trigger
            if (currentPlayerTileCoords.mCol == 24 && currentPlayerTileCoords.mRow == 34)
            {
                if (pokedexStateComponent.mPokedexUnlocked == false)
                {
                    StartOverworldFlowState<ViridianRudeGuyOverworldFlowState>(mWorld);
                }                
            }
            // Gym trigger
            else if (currentPlayerTileCoords.mCol == 37 && currentPlayerTileCoords.mRow == 35)
            {
                StartOverworldFlowState<ViridianGymLockedOverworldFlowState>(mWorld);
            }            
        }
        else if (lastNpcSpokenToLevelIndex == 4)
        {
            if (pokedexStateComponent.mPokedexUnlocked == false)
            {
                StartOverworldFlowState<ViridianRudeGuyOverworldFlowState>(mWorld);
            }
        }
        else if (lastNpcSpokenToLevelIndex == 9)
        {
            StartOverworldFlowState<ViridianCaterpieWeedleGuyOverworldFlowState>(mWorld);
        }        
    }
    else if (activeLevelComponent.mActiveLevelNameId == StringId("in_viridian_poke_center"))
    {
        // Joey flow
        if (lastNpcSpokenToLevelIndex == 2)
        {
            StartOverworldFlowState<PokeCenterHealingIntroDialogFlowState>(mWorld);
        }
    }

}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
