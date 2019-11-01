//
//  EncounterUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/11/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "EncounterUtils.h"
#include "../components/LevelModelComponent.h"
#include "../../common/utils/FileUtils.h"
#include "../../common/utils/Logging.h"
#include "../../common/utils/MathUtils.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <json.hpp>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

bool DoesLevelHaveWildEncounters(const LevelModelComponent& levelModelComponent)
{
    return levelModelComponent.mWildEncounters.size() > 0;
}

bool WildEncounterRNGTriggered(const LevelModelComponent& levelModelComponent)
{
    return levelModelComponent.mEncounterDensity >= math::RandomInt(1, 256);
}

const WildEncounterInfo& SelectRandomWildEncounter(const LevelModelComponent& levelModelComponent)
{
    const auto randomEncounterIndex  = math::RandomInt(1, 256);
    auto encounterRateAccumulator    = 0;

    for (const auto& encounterInfo: levelModelComponent.mWildEncounters)
    {
        encounterRateAccumulator += encounterInfo.mRate;
        if (randomEncounterIndex <= encounterRateAccumulator)
        {            
            return encounterInfo;
        }
    }

    assert(false && "Encounter rate accumulator larger than sum of encounter rates");
    return levelModelComponent.mWildEncounters[0];
}

std::vector<StringId> FindAllLevelNamesWherePokemonCanBeEncountered(const StringId pokemonName)
{
    std::vector<StringId> result;

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    const auto allEncounterRateFileNames = GetAllFilenamesInDirectory(ResourceLoadingService::ENCOUNTER_DATA_ROOT);
    for (const auto& fileName : allEncounterRateFileNames)
    {
        const auto filePath = ResourceLoadingService::ENCOUNTER_DATA_ROOT + fileName;

        // Check in case the resource has already been loaded
        if (resourceLoadingService.HasLoadedResource(filePath) == false)
        {
            resourceLoadingService.LoadResource(filePath);
        }

        // Get the data file handle
        const auto& encounterDataFileResource = resourceLoadingService.GetResource<DataFileResource>(filePath);

        // Create encounter json object
        const auto encounterJson = nlohmann::json::parse(encounterDataFileResource.GetContents());

        // Extract encounters
        for (const auto& encounterInfoJsonObject : encounterJson["encounters"])
        {            
            if (pokemonName == StringId(encounterInfoJsonObject["pokemon"].get<std::string>()))
            {
                result.push_back(StringSplit(fileName, '.')[0]);
                break;
            }            
        }

        resourceLoadingService.UnloadResource(filePath);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
