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
#include "../../common/flowstates/PCIntroDialogOverworldFlowState.h"
#include "../../common/flowstates/PewterBrockGuideOverworldFlowState.h"
#include "../../common/flowstates/PewterFarmerDialogOverworldFlowState.h"
#include "../../common/flowstates/PewterMuseumGuideOverworldFlowState.h"
#include "../../common/flowstates/PokeCenterHealingIntroDialogOverworldFlowState.h"
#include "../../common/flowstates/PokeMartIntroDialogOverworldFlowState.h"
#include "../../common/flowstates/TownMapOverworldFlowState.h"
#include "../../common/flowstates/ViridianCaterpieWeedleGuyOverworldFlowState.h"
#include "../../common/flowstates/ViridianGymLockedOverworldFlowState.h"
#include "../../common/flowstates/ViridianRudeGuyOverworldFlowState.h"
#include "../../common/flowstates/ViridianSchoolBlackboardOverworldFlowState.h"
#include "../../common/flowstates/ViridianSchoolBookOverworldFlowState.h"
#include "../../common/utils/OSMessageBox.h"
#include "../../resources/ResourceLoadingService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#define CreateFlowStateFactory(StateName) [this]()\
 ->  std::unique_ptr<BaseFlowState>               \
{                                                 \
	return std::make_unique<StateName>(mWorld);   \
}

#define RegisterNamedFlowState(StateName) mNamedFlowStatesFactory[StringId(#StateName)] = CreateFlowStateFactory(StateName);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string EXPOSED_NAMED_FLOW_STATES_FILE_NAME = "exposed_named_flow_states.json";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OverworldFlowControllerSystem::OverworldFlowControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    InitializeOverworldFlowState();
	RegisterNamedFlowStateFactories();
	UpdateExposedNamedFlowStatesFile();
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

void OverworldFlowControllerSystem::RegisterNamedFlowStateFactories()
{
	// Here we expose only the overworld states that we need to expose to the level editor

	RegisterNamedFlowState(PCIntroDialogOverworldFlowState);
	RegisterNamedFlowState(TownMapOverworldFlowState);
	RegisterNamedFlowState(ViridianRudeGuyOverworldFlowState);
	RegisterNamedFlowState(ViridianGymLockedOverworldFlowState);
	RegisterNamedFlowState(ViridianCaterpieWeedleGuyOverworldFlowState);
	RegisterNamedFlowState(PokeCenterHealingIntroDialogOverworldFlowState);	
	RegisterNamedFlowState(PokeMartIntroDialogOverworldFlowState);	
	RegisterNamedFlowState(ViridianSchoolBlackboardOverworldFlowState);	
	RegisterNamedFlowState(ViridianSchoolBookOverworldFlowState);		
	RegisterNamedFlowState(PewterBrockGuideOverworldFlowState);
	RegisterNamedFlowState(PewterMuseumGuideOverworldFlowState);
	RegisterNamedFlowState(PewterFarmerDialogOverworldFlowState);	
}

void OverworldFlowControllerSystem::UpdateExposedNamedFlowStatesFile() const
{	
	std::stringstream namedStatesFileString;
	namedStatesFileString << "{\"named_states\": [";	
	auto commaToggle = false;
	for (const auto& namedStateFactoryEntry : mNamedFlowStatesFactory)
	{
		if (!commaToggle)
		{
			commaToggle = true;
		}
		else
		{
			namedStatesFileString << ",";
		}

		namedStatesFileString << "\"" << namedStateFactoryEntry.first.GetString() << "\"";
	}

	namedStatesFileString << "]}";
	ResourceLoadingService::GetInstance().WriteStringToFile(namedStatesFileString.str(), ResourceLoadingService::RES_DATA_ROOT + EXPOSED_NAMED_FLOW_STATES_FILE_NAME);
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

    if (activeLevelComponent.mActiveLevelNameId == StringId("in_players_home_top"))
    {
        if (lastNpcSpokenToLevelIndex == 0)
        {
            StartOverworldFlowState<PCIntroDialogOverworldFlowState>(mWorld);
        }
    }
    else if (activeLevelComponent.mActiveLevelNameId == StringId("in_rivals_home"))
    {
        if (lastNpcSpokenToLevelIndex == 0)
        {
            StartOverworldFlowState<TownMapOverworldFlowState>(mWorld);
        }
    }
    else if (activeLevelComponent.mActiveLevelNameId == StringId("viridian_city"))
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
        else if (lastNpcSpokenToLevelIndex == 5)
        {
            if (pokedexStateComponent.mPokedexUnlocked == false)
            {
                StartOverworldFlowState<ViridianRudeGuyOverworldFlowState>(mWorld);
            }
        }
        else if (lastNpcSpokenToLevelIndex == 10)
        {
            StartOverworldFlowState<ViridianCaterpieWeedleGuyOverworldFlowState>(mWorld);
        }        
    }
    else if (activeLevelComponent.mActiveLevelNameId == StringId("in_viridian_poke_center"))
    {
        // Joey flow
        if (lastNpcSpokenToLevelIndex == 2)
        {
            StartOverworldFlowState<PokeCenterHealingIntroDialogOverworldFlowState>(mWorld);
        }
        // PC flow
        else if (lastNpcSpokenToLevelIndex == 7)
        {
            StartOverworldFlowState<PCIntroDialogOverworldFlowState>(mWorld);
        }
    }
    else if (activeLevelComponent.mActiveLevelNameId == StringId("in_viridian_poke_mart"))
    {
        // Mart flow
        if (lastNpcSpokenToLevelIndex == 1 || lastNpcSpokenToLevelIndex == 7 || lastNpcSpokenToLevelIndex == 9)
        {
            StartOverworldFlowState<PokeMartIntroDialogOverworldFlowState>(mWorld);
        }
    }
    else if (activeLevelComponent.mActiveLevelNameId == StringId("in_viridian_nickname_family"))
    {
        // Town Map Flow
        if (lastNpcSpokenToLevelIndex == 0)
        {
            StartOverworldFlowState<TownMapOverworldFlowState>(mWorld);
        }
    }
    else if (activeLevelComponent.mActiveLevelNameId == StringId("in_viridian_school"))
    {
        // Blackboard flow
        if (lastNpcSpokenToLevelIndex == 0)
        {
            StartOverworldFlowState<ViridianSchoolBlackboardOverworldFlowState>(mWorld);
        }
        // Book lesson flow
        else if (lastNpcSpokenToLevelIndex == 3)
        {
            StartOverworldFlowState<ViridianSchoolBookOverworldFlowState>(mWorld);
        }
    }
    else if (activeLevelComponent.mActiveLevelNameId == StringId("pewter_city"))
    {
        if (flowStartedByTileTrigger && IsAnyOverworldFlowCurrentlyRunning(mWorld) == false)
        {
            StartOverworldFlowState<PewterBrockGuideOverworldFlowState>(mWorld);
        }
        else
        {
            // Brock guide flow
            if (lastNpcSpokenToLevelIndex == 2 && IsAnyOverworldFlowCurrentlyRunning(mWorld) == false)
            {
                StartOverworldFlowState<PewterBrockGuideOverworldFlowState>(mWorld);
            }
            // Museum guide flow
            else if (lastNpcSpokenToLevelIndex == 5)
            {
                StartOverworldFlowState<PewterMuseumGuideOverworldFlowState>(mWorld);
            }
            // Farmer flow
            else if (lastNpcSpokenToLevelIndex == 10)
            {
                StartOverworldFlowState<PewterFarmerDialogOverworldFlowState>(mWorld);
            }
        }
    }
    
    auto& overworldFlowStateComponent = mWorld.GetSingletonComponent<OverworldFlowStateSingletonComponent>();
    if (overworldFlowStateComponent.mFlowStateManager.HasActiveFlowState() == false)
    {
        ShowMessageBox(MessageBoxType::WARNING, "No flow activated", "Flow not hooked properly");
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
