//
//  PersistenceUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 21/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PersistenceUtils.h"
#include "PokemonItemsUtils.h"
#include "PokemonMoveUtils.h"
#include "PokedexUtils.h"
#include "PokemonUtils.h"
#include "../components/DirectionComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokedexStateSingletonComponent.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/utils/LevelLoadingUtils.h"
#include "../../overworld/utils/OverworldCharacterLoadingUtils.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../sound/SoundService.h"

#include <json.hpp>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string SAVE_FILE_NAME = "user/save.json";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<Pokemon> CreatePokemonFromJson(const nlohmann::basic_json<>& pokemonJsonEntry, ecs::World& world);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void SaveGame(const ecs::World& world)
{
    const auto& playerEntityId        = GetPlayerEntityId(world);
    const auto& playerStateComponent  = world.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activeLevelComponent  = world.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& pokedexStateComponent = world.GetSingletonComponent<PokedexStateSingletonComponent>();
    const auto& movementComponent     = world.GetComponent<MovementStateComponent>(playerEntityId);
    const auto& directionComponent    = world.GetComponent<DirectionComponent>(playerEntityId);
    
    std::stringstream saveFileString;
    saveFileString << "{\n";
    saveFileString << "    \"player_name\": \"" << playerStateComponent.mPlayerTrainerName.GetString() << "\",\n";
    saveFileString << "    \"rival_name\": \"" << playerStateComponent.mRivalName.GetString() << "\",\n";
    saveFileString << "    \"home_name\": \"" << playerStateComponent.mHomeLevelName.GetString() << "\",\n";
    saveFileString << "    \"home_col\": " << playerStateComponent.mHomeLevelOccupiedCol << ",\n";
    saveFileString << "    \"home_row\": " << playerStateComponent.mHomeLevelOccupiedRow << ",\n";
    saveFileString << "    \"milestones\": " << playerStateComponent.mMilestones << ",\n";
    saveFileString << "    \"current_level_name\": \"" << activeLevelComponent.mActiveLevelNameId.GetString() << "\",\n";
    saveFileString << "    \"current_game_col\": " << movementComponent.mCurrentCoords.mCol << ",\n";
    saveFileString << "    \"current_game_row\": " << movementComponent.mCurrentCoords.mRow << ",\n";
    saveFileString << "    \"current_direction\": " << static_cast<int>(directionComponent.mDirection) << ",\n";
    saveFileString << "    \"seconds_played\": " << playerStateComponent.mSecondsPlayed << ",\n";
    saveFileString << "    \"poke_dollars\": " << playerStateComponent.mPokeDollarCredits << ",\n";
    saveFileString << "    \"trainer_id\": " << playerStateComponent.mTrainerId << ",\n"; 
    saveFileString << "    \"has_active_museum_ticket\": " << (playerStateComponent.mHasPurchasedMuseumTicket  ? "true" : "false") << ",\n";
    
    // Serialize defeated npc entries
    saveFileString << "    \"defeated_npc_entries\": [";
    for (auto i = 0U; i < playerStateComponent.mDefeatedNpcEntries.size(); ++i)
    {
        const auto& defeatedNpcEntry = playerStateComponent.mDefeatedNpcEntries[i];
        
        if (i != 0)
        {
            saveFileString << ", ";
        }
        
        saveFileString << "{ \"level_name\": \"" << defeatedNpcEntry.mNpcLevelName.GetString() << "\", \"level_index\": " << defeatedNpcEntry.mNpcLevelIndex << " }";
    }
    saveFileString << "],\n";
    
    // Serialize collected npcs (pokeballs in overworld)
    saveFileString << "    \"collected_npc_item_entries\": [";
    for (auto i = 0U; i < playerStateComponent.mCollectedNpcItemEntries.size(); ++i)
    {
        const auto& collectedNpcItemEntry = playerStateComponent.mCollectedNpcItemEntries[i];
        
        if (i != 0)
        {
            saveFileString << ", ";
        }
        
        saveFileString << "{ \"level_name\": \"" << collectedNpcItemEntry.mNpcLevelName.GetString() << "\", \"level_index\": " << collectedNpcItemEntry.mNpcLevelIndex << " }";
    }
    saveFileString << "],\n";
    
    // Serialize npc entries that have their items given to player already
    saveFileString << "    \"collected_npc_item_non_destructible_entries\": [";
    for (auto i = 0U; i < playerStateComponent.mCollectedItemNonDestructibleNpcEntries.size(); ++i)
    {
        const auto& collectedNonDestructibleNpcEntry = playerStateComponent.mCollectedItemNonDestructibleNpcEntries[i];
        
        if (i != 0)
        {
            saveFileString << ", ";
        }
        
        saveFileString << "{ \"level_name\": \"" << collectedNonDestructibleNpcEntry.mNpcLevelName.GetString() << "\", \"level_index\": " << collectedNonDestructibleNpcEntry.mNpcLevelIndex << " }";
    }
    saveFileString << "],\n";
    
    // Serialize bag items
    saveFileString << "    \"bag\": [";
    for (auto i = 0U; i < playerStateComponent.mPlayerBag.size(); ++i)
    {
        const auto& bagItemEntry = playerStateComponent.mPlayerBag[i];
        
        if (i != 0)
        {
            saveFileString << ", ";
        }
        
        saveFileString << "{ \"item_name\": \"" << bagItemEntry.mItemName.GetString() << "\", \"item_quantity\": " << bagItemEntry.mQuantity << " }";
    }
    saveFileString << "],\n";
    
    // Serialize pokedex state
    saveFileString << "    \"pokedex\": [";
    for (auto i = 0U; i < pokedexStateComponent.mPokedexEntries.size(); ++i)
    {
        
        if (i != 0)
        {
            saveFileString << ", ";
        }
        
        saveFileString << static_cast<int>(pokedexStateComponent.mPokedexEntries[i]);
    }
    saveFileString << "],\n";
    
    // Serialize boxed pokemon
    saveFileString << "    \"box_pokemon\": [";
    for (auto i = 0U; i < playerStateComponent.mPlayerBoxedPokemon.size(); ++i)
    {
        const auto& pokemon = *playerStateComponent.mPlayerBoxedPokemon[i];
        
        if (i != 0)
        {
            saveFileString << ", ";
        }
        
        saveFileString << "{ \"species_name\": \"" << pokemon.mBaseSpeciesStats.mSpeciesName.GetString() << "\", ";
        saveFileString << "\"nickname\": \"" << pokemon.mName.GetString() << "\", ";
        saveFileString << "\"level\": " << pokemon.mLevel << ", ";
        saveFileString << "\"current_hp\": " << pokemon.mHp << ", ";
        saveFileString << "\"xp_points\": " << pokemon.mXpPoints << ", ";
        saveFileString << "\"status\": " << static_cast<int>(pokemon.mStatus) << ", ";
        saveFileString << "\"rounds_to_awake\": " << pokemon.mNumberOfRoundsUntilSleepEnds << ", ";
        saveFileString << "\"hp_iv\": " << pokemon.mHpIv << ", ";
        saveFileString << "\"attack_iv\": " << pokemon.mAttackIv << ", ";
        saveFileString << "\"defense_iv\": " << pokemon.mDefenseIv << ", ";
        saveFileString << "\"speed_iv\": " << pokemon.mSpeedIv << ", ";
        saveFileString << "\"special_iv\": " << pokemon.mSpecialIv << ", ";
        saveFileString << "\"hp_ev\": " << pokemon.mHpEv << ", ";
        saveFileString << "\"attack_ev\": " << pokemon.mAttackEv << ", ";
        saveFileString << "\"defense_ev\": " << pokemon.mDefenseEv << ", ";
        saveFileString << "\"speed_ev\": " << pokemon.mSpeedEv << ", ";
        saveFileString << "\"special_ev\": " << pokemon.mSpecialEv << ", ";
        saveFileString << "\"moveset\": [";
        
        for (auto j = 0U; j < pokemon.mMoveSet.size(); ++j)
        {
            if (pokemon.mMoveSet[j] == nullptr)
            {
                break;
            }
            
            if (j != 0)
            {
                saveFileString << ", ";
            }
            
            const auto& move = *pokemon.mMoveSet[j];
            saveFileString << "{ \"move_name\": \"" << move.mName.GetString() << "\", \"pp_left\": " << move.mPowerPointsLeft << " }";
        }
        saveFileString << "]";
        
        saveFileString << " }";
    }
    saveFileString << "],\n";
    
    // Serialize party pokemon
    saveFileString << "    \"pokemon\": [";
    for (auto i = 0U; i < playerStateComponent.mPlayerPokemonRoster.size(); ++i)
    {
        const auto& pokemon = *playerStateComponent.mPlayerPokemonRoster[i];
        
        if (i != 0)
        {
            saveFileString << ", ";
        }
        
        saveFileString << "{ \"species_name\": \"" << pokemon.mBaseSpeciesStats.mSpeciesName.GetString() << "\", ";
        saveFileString << "\"nickname\": \"" << pokemon.mName.GetString() << "\", ";
        saveFileString << "\"level\": " << pokemon.mLevel << ", ";
        saveFileString << "\"current_hp\": " << pokemon.mHp << ", ";
        saveFileString << "\"xp_points\": " << pokemon.mXpPoints << ", ";
        saveFileString << "\"status\": " << static_cast<int>(pokemon.mStatus) << ", ";
        saveFileString << "\"rounds_to_awake\": " << pokemon.mNumberOfRoundsUntilSleepEnds << ", ";
        saveFileString << "\"hp_iv\": " << pokemon.mHpIv << ", ";
        saveFileString << "\"attack_iv\": " << pokemon.mAttackIv << ", ";
        saveFileString << "\"defense_iv\": " << pokemon.mDefenseIv << ", ";
        saveFileString << "\"speed_iv\": " << pokemon.mSpeedIv << ", ";
        saveFileString << "\"special_iv\": " << pokemon.mSpecialIv << ", ";
        saveFileString << "\"hp_ev\": " << pokemon.mHpEv << ", ";
        saveFileString << "\"attack_ev\": " << pokemon.mAttackEv << ", ";
        saveFileString << "\"defense_ev\": " << pokemon.mDefenseEv << ", ";
        saveFileString << "\"speed_ev\": " << pokemon.mSpeedEv << ", ";
        saveFileString << "\"special_ev\": " << pokemon.mSpecialEv << ", ";
        saveFileString << "\"moveset\": [";
        
        for (auto j = 0U; j < pokemon.mMoveSet.size(); ++j)
        {
            if (pokemon.mMoveSet[j] == nullptr)
            {
                break;
            }
            
            if (j != 0)
            {
                saveFileString << ", ";
            }
            
            const auto& move = *pokemon.mMoveSet[j];
            saveFileString << "{ \"move_name\": \"" << move.mName.GetString() << "\", \"pp_left\": " << move.mPowerPointsLeft << " }";
        }
        saveFileString << "]";
        
        saveFileString << " }";
    }
    saveFileString << "]\n";
    
    saveFileString << "}";
    
    ResourceLoadingService::GetInstance().WriteStringToFile(saveFileString.str(), ResourceLoadingService::RES_DATA_ROOT + SAVE_FILE_NAME);
}

void RestoreGameStateFromSaveFile(ecs::World& world)
{
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    
    // Get level data file resource
    const auto saveFilePath = ResourceLoadingService::RES_DATA_ROOT + SAVE_FILE_NAME;
    resourceLoadingService.LoadResource(saveFilePath);
    const auto& saveFileResource = resourceLoadingService.GetResource<DataFileResource>(saveFilePath);
    
    // Parse level json
    const auto saveJson = nlohmann::json::parse(saveFileResource.GetContents());
    
    auto playerStateComponent = std::make_unique<PlayerStateSingletonComponent>();
    playerStateComponent->mPlayerTrainerName        = StringId(saveJson["player_name"].get<std::string>());
    playerStateComponent->mRivalName                = StringId(saveJson["rival_name"].get<std::string>());
    playerStateComponent->mMilestones               = saveJson["milestones"].get<unsigned long>();
    playerStateComponent->mHomeLevelName            = StringId(saveJson["home_name"].get<std::string>());
    playerStateComponent->mHomeLevelOccupiedCol     = saveJson["home_col"].get<int>();
    playerStateComponent->mHomeLevelOccupiedRow     = saveJson["home_row"].get<int>();
    playerStateComponent->mSecondsPlayed            = saveJson["seconds_played"].get<int>();
    playerStateComponent->mPokeDollarCredits        = saveJson["poke_dollars"].get<int>();
    playerStateComponent->mTrainerId                = saveJson["trainer_id"].get<int>();

    // Checks to not break backwards compatibility with newer save file changes
    if (saveJson.count("has_active_museum_ticket") > 0)
    {
        playerStateComponent->mHasPurchasedMuseumTicket = saveJson["has_active_museum_ticket"].get<bool>();
    }
    
    for (const auto& defeatedNpcEntry: saveJson["defeated_npc_entries"])
    {
        playerStateComponent->mDefeatedNpcEntries.emplace_back
        (
            StringId(defeatedNpcEntry["level_name"].get<std::string>()),
            defeatedNpcEntry["level_index"].get<int>()
        );
    }
    
    for (const auto& collectedNpcItemEntry: saveJson["collected_npc_item_entries"])
    {
        playerStateComponent->mCollectedNpcItemEntries.emplace_back
        (
            StringId(collectedNpcItemEntry["level_name"].get<std::string>()),
            collectedNpcItemEntry["level_index"].get<int>()
        );
    }
    
    for (const auto& collectedNpcNonDestructibleEntry: saveJson["collected_npc_item_non_destructible_entries"])
    {
        playerStateComponent->mCollectedItemNonDestructibleNpcEntries.emplace_back
        (
            StringId(collectedNpcNonDestructibleEntry["level_name"].get<std::string>()),
            collectedNpcNonDestructibleEntry["level_index"].get<int>()
        );
    }
    
    for (const auto& pokemonEntry: saveJson["pokemon"])
    {
        playerStateComponent->mPlayerPokemonRoster.push_back(CreatePokemonFromJson(pokemonEntry, world));
    }
    
    for (const auto& pokemonEntry: saveJson["box_pokemon"])
    {
        playerStateComponent->mPlayerBoxedPokemon.push_back(CreatePokemonFromJson(pokemonEntry, world));
    }
    
    world.SetSingletonComponent<PlayerStateSingletonComponent>(std::move(playerStateComponent));
    
    InitializePlayerBag(world);
        
    for (const auto& bagItemEntry: saveJson["bag"])
    {
        AddItemToBag
        (
            StringId(bagItemEntry["item_name"].get<std::string>()),
            world,
            bagItemEntry["item_quantity"].get<int>()
        );
    }
    
    auto pokedexIdCounter = 1;
    for (const auto& pokedexEntry : saveJson["pokedex"])
    {
        ChangePokedexEntryForPokemon(GetPokemonNameFromPokedexId(pokedexIdCounter++, world), static_cast<PokedexEntryType>(pokedexEntry.get<int>()), world);
    }

    const auto levelEntityId  = LoadAndCreateLevelByName(StringId(saveJson["current_level_name"].get<std::string>()), world);
    auto& levelModelComponent = world.GetComponent<LevelModelComponent>(levelEntityId);
    
    auto activeLevelComponent = std::make_unique<ActiveLevelSingletonComponent>();
    activeLevelComponent->mActiveLevelNameId = levelModelComponent.mLevelName;
    world.SetSingletonComponent<ActiveLevelSingletonComponent>(std::move(activeLevelComponent));
    
    CreatePlayerOverworldSprite
    (
        levelEntityId,
        static_cast<Direction>(saveJson["current_direction"].get<int>()),
        saveJson["current_game_col"].get<int>(),
        saveJson["current_game_row"].get<int>(),
        world
    );
    
    SoundService::GetInstance().PlayMusic(levelModelComponent.mLevelMusicTrackName);
}

bool DoesSaveFileExist()
{
    return ResourceLoadingService::GetInstance().DoesFileExist(ResourceLoadingService::RES_DATA_ROOT + SAVE_FILE_NAME);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Pokemon> CreatePokemonFromJson(const nlohmann::basic_json<>& pokemonJsonEntry, ecs::World& world)
{
    auto pokemonInstance = CreatePokemon
    (
        StringId(pokemonJsonEntry["species_name"].get<std::string>()),
        pokemonJsonEntry["level"].get<int>(),
        false,
        world
    );
    
    pokemonInstance->mName = StringId(pokemonJsonEntry["nickname"].get<std::string>());
    pokemonInstance->mXpPoints = pokemonJsonEntry["xp_points"].get<int>();
    pokemonInstance->mStatus = static_cast<PokemonStatus>(pokemonJsonEntry["status"].get<int>());
    pokemonInstance->mNumberOfRoundsUntilSleepEnds = pokemonJsonEntry["rounds_to_awake"].get<int>();
    
    pokemonInstance->mHpIv      = pokemonJsonEntry["hp_iv"].get<int>();
    pokemonInstance->mAttackIv  = pokemonJsonEntry["attack_iv"].get<int>();
    pokemonInstance->mDefenseIv = pokemonJsonEntry["defense_iv"].get<int>();
    pokemonInstance->mSpeedIv   = pokemonJsonEntry["speed_iv"].get<int>();
    pokemonInstance->mSpecialIv = pokemonJsonEntry["special_iv"].get<int>();
    
    pokemonInstance->mHpEv      = pokemonJsonEntry["hp_ev"].get<int>();
    pokemonInstance->mAttackEv  = pokemonJsonEntry["attack_ev"].get<int>();
    pokemonInstance->mDefenseEv = pokemonJsonEntry["defense_ev"].get<int>();
    pokemonInstance->mSpeedEv   = pokemonJsonEntry["speed_ev"].get<int>();
    pokemonInstance->mSpecialEv = pokemonJsonEntry["special_ev"].get<int>();
    
    const auto& baseSpeciesStats = pokemonInstance->mBaseSpeciesStats;
    
    pokemonInstance->mMaxHp   = CalculateHpStat(pokemonInstance->mLevel, baseSpeciesStats.mHp, pokemonInstance->mHpIv, pokemonInstance->mHpEv);
    pokemonInstance->mAttack  = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseSpeciesStats.mAttack, pokemonInstance->mAttackIv, pokemonInstance->mAttackEv);
    pokemonInstance->mDefense = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseSpeciesStats.mDefense, pokemonInstance->mDefenseIv, pokemonInstance->mDefenseEv);
    pokemonInstance->mSpeed   = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseSpeciesStats.mSpeed, pokemonInstance->mSpeedIv, pokemonInstance->mSpeedEv);
    pokemonInstance->mSpecial = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseSpeciesStats.mSpecial, pokemonInstance->mSpecialIv, pokemonInstance->mSpecialEv);
    
    pokemonInstance->mHp = pokemonJsonEntry["current_hp"].get<int>();
    
    int moveIndex = 0;
    for (const auto& moveEntry: pokemonJsonEntry["moveset"])
    {
        const auto& moveName = StringId(moveEntry["move_name"].get<std::string>());
        const auto& ppLeft = moveEntry["pp_left"].get<int>();
                
        AddMoveToIndex(moveName, moveIndex, world, *pokemonInstance);

        pokemonInstance->mMoveSet[moveIndex++]->mPowerPointsLeft = ppLeft;
    }        
    
    return pokemonInstance;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
