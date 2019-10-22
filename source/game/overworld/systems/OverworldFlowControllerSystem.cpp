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
#include "../../common/flowstates/OakDialogOverworldFlowState.h"
#include "../../common/flowstates/PCIntroDialogOverworldFlowState.h"
#include "../../common/flowstates/PewterBrockGuideOverworldFlowState.h"
#include "../../common/flowstates/PewterFarmerDialogOverworldFlowState.h"
#include "../../common/flowstates/PewterMuseumGuideOverworldFlowState.h"
#include "../../common/flowstates/PikachuCatchIntroSequenceOverworldFlowState.h"
#include "../../common/flowstates/PokeCenterHealingIntroDialogOverworldFlowState.h"
#include "../../common/flowstates/PokeMartIntroDialogOverworldFlowState.h"
#include "../../common/flowstates/TownMapOverworldFlowState.h"
#include "../../common/flowstates/ViridianCaterpieWeedleGuyOverworldFlowState.h"
#include "../../common/flowstates/ViridianGymLockedOverworldFlowState.h"
#include "../../common/flowstates/ViridianRudeGuyOverworldFlowState.h"
#include "../../common/flowstates/ViridianSchoolBlackboardOverworldFlowState.h"
#include "../../common/flowstates/ViridianSchoolBookOverworldFlowState.h"
#include "../../common/flowstates/ViridianOaksParcelOverworldFlowState.h"
#include "../../common/flowstates/RivalRoute22EncounterOverworldFlowState.h"
#include "../../common/utils/OSMessageBox.h"
#include "../../common/utils/StringUtils.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../resources/DataFileResource.h"

#include <json.hpp>

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
const std::string OVERWORLD_FLOW_STATE_MAP_FILE_NAME  = "overworld_flow_state_map.json";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OverworldFlowControllerSystem::OverworldFlowControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    RegisterNamedFlowStateFactories();
    UpdateExposedNamedFlowStatesFile();
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
    auto overworldFlorStateComponent = std::make_unique<OverworldFlowStateSingletonComponent>();

    // Load flow state map file
    const auto overworldFlowStateMapResourceId = ResourceLoadingService::GetInstance().
        LoadResource(ResourceLoadingService::RES_DATA_ROOT + OVERWORLD_FLOW_STATE_MAP_FILE_NAME);

    const auto& overworldFlowStateMapDataResource = ResourceLoadingService::GetInstance().
        GetResource<DataFileResource>(overworldFlowStateMapResourceId);

    const auto flowStateMapJson = nlohmann::json::parse(overworldFlowStateMapDataResource.GetContents());

    // Parse flow state npc entries
    for (const auto& npcFlowStateEntry : flowStateMapJson["npc_flow_states"])
    {
        overworldFlorStateComponent->mNpcFlowStateMapEntries.emplace_back
        (
            StringId(npcFlowStateEntry["level_name"].get<std::string>()),
            StringId(npcFlowStateEntry["flow_state_name"].get<std::string>()),
            npcFlowStateEntry["npc_level_index"].get<int>()
        );
    }

    // Parse trigger entries
    for (const auto& flowTriggerEntry : flowStateMapJson["trigger_flow_states"])
    {
        overworldFlorStateComponent->mTriggerFlowStateMapEntries.emplace_back
        (
            StringId(flowTriggerEntry["level_name"].get<std::string>()),
            StringId(flowTriggerEntry["flow_state_name"].get<std::string>()),
            flowTriggerEntry["level_col"].get<int>(),
            flowTriggerEntry["level_row"].get<int>()
        );
    }    

    mWorld.SetSingletonComponent<OverworldFlowStateSingletonComponent>(std::move(overworldFlorStateComponent));
}

void OverworldFlowControllerSystem::RegisterNamedFlowStateFactories()
{
    // Here we expose only the overworld states that we need the level editor to have visibility of
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
    RegisterNamedFlowState(ViridianOaksParcelOverworldFlowState);
    RegisterNamedFlowState(OakDialogOverworldFlowState);
    RegisterNamedFlowState(RivalRoute22EncounterOverworldFlowState);
    RegisterNamedFlowState(PikachuCatchIntroSequenceOverworldFlowState);
}

void OverworldFlowControllerSystem::UpdateExposedNamedFlowStatesFile() const
{    
    std::vector<std::string> stateNames;
    for (const auto& namedStateEntry : mNamedFlowStatesFactory)
    {
        stateNames.emplace_back(namedStateEntry.first.GetString());
    }
    std::sort(stateNames.begin(), stateNames.end());

    std::stringstream namedStatesFileString;
    namedStatesFileString << "{\n";
    namedStatesFileString << "\t\"named_states\": [";    

    auto commaToggle = false;
    for (const auto& stateName : stateNames)
    {
        if (!commaToggle)
        {
            commaToggle = true;
        }
        else
        {
            namedStatesFileString << ",";
        }

        namedStatesFileString << "\"" << stateName << "\"";
    }

    namedStatesFileString << "]\n";
    namedStatesFileString << "}";

    ResourceLoadingService::GetInstance().WriteStringToFile(namedStatesFileString.str(), ResourceLoadingService::RES_DATA_ROOT + EXPOSED_NAMED_FLOW_STATES_FILE_NAME);
}

void OverworldFlowControllerSystem::DetermineWhichFlowToStart() const
{
    const auto& activeLevelComponent        = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& playerStateComponent        = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activeLevelModelComponent   = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
    const auto& playerMovementComponent     = mWorld.GetComponent<MovementStateComponent>(GetPlayerEntityId(mWorld));
    auto& overworldFlowStateComponent       = mWorld.GetSingletonComponent<OverworldFlowStateSingletonComponent>();
    const auto& currentPlayerTileCoords     = playerMovementComponent.mCurrentCoords;
    const auto& currentPlayerTile           = GetTile(playerMovementComponent.mCurrentCoords, activeLevelModelComponent.mLevelTilemap);
    const auto currentLevelCol              = currentPlayerTileCoords.mCol;
    const auto currentLevelRow              = currentPlayerTileCoords.mRow;
    const auto currentLevelName             = activeLevelComponent.mActiveLevelNameId;
    const auto lastNpcSpokenToLevelIndex    = playerStateComponent.mLastNpcLevelIndexSpokenTo;
    const auto flowStartedByTileTrigger     = currentPlayerTile.mTileTrait == TileTrait::FLOW_TRIGGER;
    
    if (IsAnyOverworldFlowCurrentlyRunning(mWorld) == false)
    {
        if (flowStartedByTileTrigger)
        {
            auto findResultIter = std::find_if
            (
                overworldFlowStateComponent.mTriggerFlowStateMapEntries.cbegin(),
                overworldFlowStateComponent.mTriggerFlowStateMapEntries.cend(),
                [currentLevelName, currentLevelCol, currentLevelRow](const TriggerFlowStateMapEntry& triggerFlowStateEntry)
                {
                return triggerFlowStateEntry.mLevelName == currentLevelName &&
                       triggerFlowStateEntry.mLevelCol  == currentLevelCol &&
                       triggerFlowStateEntry.mLevelRow  == currentLevelRow;
                }
            );
            
            if (findResultIter != overworldFlowStateComponent.mTriggerFlowStateMapEntries.cend())
            {
                auto flowState = mNamedFlowStatesFactory.at(findResultIter->mFlowName)();
                overworldFlowStateComponent.mFlowStateManager.SetActiveFlowState(std::move(flowState));
            }
        }
        else
        {
            auto findResultIter = std::find_if
            (
                overworldFlowStateComponent.mNpcFlowStateMapEntries.cbegin(),
                overworldFlowStateComponent.mNpcFlowStateMapEntries.cend(),
                [currentLevelName, lastNpcSpokenToLevelIndex](const NpcFlowStateMapEntry& npcFlowStateEntry)
                {
                    return npcFlowStateEntry.mLevelName     == currentLevelName &&
                           npcFlowStateEntry.mNpcLevelIndex == lastNpcSpokenToLevelIndex;
                }
            );
            
            if (findResultIter != overworldFlowStateComponent.mNpcFlowStateMapEntries.cend())
            {
                auto flowState = mNamedFlowStatesFactory.at(findResultIter->mFlowName)();
                overworldFlowStateComponent.mFlowStateManager.SetActiveFlowState(std::move(flowState));
            }
        }
    }

    if (overworldFlowStateComponent.mFlowStateManager.HasActiveFlowState() == false)
    {
        ShowMessageBox(MessageBoxType::WARNING, "No flow activated", "Flow not hooked properly");
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
