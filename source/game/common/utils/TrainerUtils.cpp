//
//  TrainerUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/07/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TrainerUtils.h"
#include "../GameConstants.h"
#include "../components/TrainersInfoStatsSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <cassert>
#include <json.hpp>
#include <unordered_map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string TRAINER_INFO_STATS_FILE_NAME = "trainer_types.json";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const TrainerInfo& GetTrainerInfo
(
    const StringId trainerSpeciesName,
    const ecs::World& world
)
{
    const auto& trainerInfoStatsComponent = world.GetSingletonComponent<TrainersInfoStatsSingletonComponent>();
    return trainerInfoStatsComponent.mTrainerInfoStats.at(trainerSpeciesName);
}

int CalculateTrainerBattleWonPayout
(
    const ecs::World& world
)
{
    //https://bulbapedia.bulbagarden.net/wiki/Prize_money
    const auto& encounterStateComponent = world.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& trainerInfo = GetTrainerInfo(encounterStateComponent.mOpponentTrainerSpeciesName, world);

    return trainerInfo.mBasePayout * encounterStateComponent.mOpponentPokemonRoster.back()->mLevel;
}

void LoadAndPopulateTrainerInfoStats
(
    TrainersInfoStatsSingletonComponent& trainerInfoStatsComponent
)
{    
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    
    // Get pokemon base stats data file resource
    const auto trainerInfoStatsFilePath = ResourceLoadingService::RES_DATA_ROOT + TRAINER_INFO_STATS_FILE_NAME;
    resourceLoadingService.LoadResource(trainerInfoStatsFilePath);
    const auto& trainerInfoStatsResource = resourceLoadingService.GetResource<DataFileResource>(trainerInfoStatsFilePath);

    // Parse base stats json
    const auto baseStatsJson = nlohmann::json::parse(trainerInfoStatsResource.GetContents());
    
    auto& trainerInfoStats = trainerInfoStatsComponent.mTrainerInfoStats;
    for (auto it = baseStatsJson.begin(); it != baseStatsJson.end(); ++it)
    {
        const auto& trainerSpeciesName = StringId(it.key());
        const auto& trainerStatsObject = it.value();

        const auto& basePayout      = trainerStatsObject["base_payout"].get<int>();        
        const auto& textureAtlasCol = trainerStatsObject["atlas_col"].get<int>();
        const auto& textureAtlasRow = trainerStatsObject["atlas_row"].get<int>();
        
        auto trainerMusicTrackName = StringId();

        if (trainerStatsObject.count("trainer_music") != 0)
        {
            trainerMusicTrackName = StringId(trainerStatsObject["trainer_music"]);
        }

        trainerInfoStats.insert(std::make_pair(trainerSpeciesName, TrainerInfo
        (
            trainerSpeciesName,
            trainerMusicTrackName,
            basePayout,
            textureAtlasCol,
            textureAtlasRow
        )));
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

