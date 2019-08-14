//
//  PokemonUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/07/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonUtils.h"
#include "PokemonMoveUtils.h"
#include "MathUtils.h"
#include "../GameConstants.h"
#include "../components/MoveStatsSingletonComponent.h"
#include "../components/PokemonBaseStatsSingletonComponent.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <cassert>
#include <cctype>
#include <json.hpp>
#include <unordered_map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string BASE_STATS_FILE_NAME = "base_stats.json";
static const std::string POKEMON_XP_GROUPS_FILE_NAME = "xp_groups.json";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static int CalculateBallShakeCountBeforePokemonBreaksFree
(
    const StringId ballNameUsed,
    const Pokemon& pokemon,
    const int fVal
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Pokemon> CreatePokemon
(
    const StringId pokemonName,
    const bool trainerOwned,
    const int pokemonLevel,
    const ecs::World& world,
    const Pokemon* const priorPokemonEvolvedFrom /* nullptr */
)
{
    const auto& baseSpeciesStats = GetPokemonBaseStats(pokemonName, world);
    auto pokemonInstance = std::make_unique<Pokemon>(pokemonName, baseSpeciesStats);

    pokemonInstance->mLevel    = pokemonLevel;    
    pokemonInstance->mXpPoints = CalculatePokemonTotalExperienceAtLevel(pokemonName, pokemonLevel, world);    

    pokemonInstance->mEvolution = nullptr;

    pokemonInstance->mMoveToBeLearned = StringId();

    pokemonInstance->mStatus = PokemonStatus::NORMAL;
    
    if (priorPokemonEvolvedFrom != nullptr)
    {
        pokemonInstance->mName = priorPokemonEvolvedFrom->mName;
    }

    if (trainerOwned)
    {
        // http://wiki.pokemonspeedruns.com/index.php?title=Pok%C3%A9mon_Red/Blue/Yellow_Trainer_AI
        pokemonInstance->mAttackIv  = 9;
        pokemonInstance->mDefenseIv = 8;
        pokemonInstance->mSpeedIv   = 8;
        pokemonInstance->mSpecialIv = 8;
    }
    else
    {
        if (priorPokemonEvolvedFrom != nullptr)
        {
            pokemonInstance->mAttackIv  = priorPokemonEvolvedFrom->mAttackIv;
            pokemonInstance->mDefenseIv = priorPokemonEvolvedFrom->mDefenseIv;
            pokemonInstance->mSpeedIv   = priorPokemonEvolvedFrom->mSpeedIv;
            pokemonInstance->mSpecialIv = priorPokemonEvolvedFrom->mSpecialIv;
        }
        else
        {
            // Calculate IVs for stats other than hp
            // https://bulbapedia.bulbagarden.net/wiki/Individual_values
            pokemonInstance->mAttackIv  = math::RandomInt(0, 15);
            pokemonInstance->mDefenseIv = math::RandomInt(0, 15);
            pokemonInstance->mSpeedIv   = math::RandomInt(0, 15);
            pokemonInstance->mSpecialIv = math::RandomInt(0, 15);
        }        
    }
        
    // Calculate Hp IVs
    // https://bulbapedia.bulbagarden.net/wiki/Individual_values
    pokemonInstance->mHpIv =
        ((pokemonInstance->mAttackIv & 0x1)  << 3) |
        ((pokemonInstance->mDefenseIv & 0x1) << 2) |
        ((pokemonInstance->mSpeedIv & 0x1)   << 1) |
        ((pokemonInstance->mSpecialIv & 0x1  << 0));

    if (priorPokemonEvolvedFrom != nullptr)
    {
        // Reset EVs
        pokemonInstance->mHpEv      = priorPokemonEvolvedFrom->mHpEv;
        pokemonInstance->mAttackEv  = priorPokemonEvolvedFrom->mAttackEv;
        pokemonInstance->mDefenseEv = priorPokemonEvolvedFrom->mDefenseEv;
        pokemonInstance->mSpeedEv   = priorPokemonEvolvedFrom->mSpeedEv;
        pokemonInstance->mSpecialEv = priorPokemonEvolvedFrom->mSpecialEv;
    }
    else
    {
        // Reset EVs
        pokemonInstance->mHpEv      = 0;
        pokemonInstance->mAttackEv  = 0;
        pokemonInstance->mDefenseEv = 0;
        pokemonInstance->mSpeedEv   = 0;
        pokemonInstance->mSpecialEv = 0;
    }    

    // Calculate Hp Stat
    pokemonInstance->mMaxHp = CalculateHpStat(pokemonInstance->mLevel, baseSpeciesStats.mHp, pokemonInstance->mHpIv, pokemonInstance->mHpEv);
    pokemonInstance->mHp    = pokemonInstance->mMaxHp;

    // Calculate other stats
    pokemonInstance->mAttack  = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseSpeciesStats.mAttack, pokemonInstance->mAttackIv, pokemonInstance->mAttackEv);
    pokemonInstance->mDefense = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseSpeciesStats.mDefense, pokemonInstance->mDefenseIv, pokemonInstance->mDefenseEv);
    pokemonInstance->mSpeed   = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseSpeciesStats.mSpeed, pokemonInstance->mSpeedIv, pokemonInstance->mSpeedEv);
    pokemonInstance->mSpecial = CalculateStatOtherThanHp(pokemonInstance->mLevel, baseSpeciesStats.mSpecial, pokemonInstance->mSpecialIv, pokemonInstance->mSpecialEv);

    // Reset encounter stage modifiers
    pokemonInstance->mAttackEncounterStage  = 0;
    pokemonInstance->mDefenseEncounterStage = 0;
    pokemonInstance->mSpeedEncounterStage   = 0;
    pokemonInstance->mSpecialEncounterStage = 0;
    pokemonInstance->mAccuracyStage         = 0;
    pokemonInstance->mEvasionStage          = 0;

    // Popuplate moveset from prior evolution
    if (priorPokemonEvolvedFrom != nullptr)
    {
        for (auto i = 0U; i < priorPokemonEvolvedFrom->mMoveSet.size(); ++i)
        {
            if (priorPokemonEvolvedFrom->mMoveSet[i] == nullptr)
                continue;

            const auto& move = priorPokemonEvolvedFrom->mMoveSet[i];

            pokemonInstance->mMoveSet[i] = std::make_unique<PokemonMoveStats>
            (
                move->mName,
                move->mType,
                move->mEffect,
                move->mPower,
                move->mAccuracy,
                move->mTotalPowerPoints
            );

            pokemonInstance->mMoveSet[i]->mPowerPointsLeft = move->mPowerPointsLeft;
        }        
    }
    // Popuplate moveset from base stats' move set
    else
    {
        auto nextInsertedMoveIndex = 0U;
        for (const auto& moveLearnInfo : baseSpeciesStats.mLearnset)
        {
            if 
            (
                moveLearnInfo.mLevelLearned > pokemonLevel || 
                DoesMovesetHaveMove(moveLearnInfo.mMoveName, pokemonInstance->mMoveSet)
            )
            {
                continue;
            }

            const auto& moveStats = GetMoveStats(moveLearnInfo.mMoveName, world);

            pokemonInstance->mMoveSet[nextInsertedMoveIndex] = std::make_unique<PokemonMoveStats>
            (
                moveStats.mName,
                moveStats.mType,
                moveStats.mEffect,
                moveStats.mPower,
                moveStats.mAccuracy,
                moveStats.mTotalPowerPoints
            );

            nextInsertedMoveIndex = (nextInsertedMoveIndex + 1) % 4;
        }
    }

    return pokemonInstance;
}

std::string GetFormattedPokemonIdString
(
    const int pokemonId
)
{
    const auto pokemonIdString = std::to_string(pokemonId);
    std::string finalIdString = "000";
    return finalIdString.replace(3 - pokemonIdString.size(), pokemonIdString.size(), pokemonIdString);
}

std::string GetFormattedPokemonStatus
(
    const int pokemonHp,
    const PokemonStatus pokemonStatus
)
{
    if (pokemonHp <= 0)
    {
        return "FNT";
    }

    switch (pokemonStatus)
    {
        case PokemonStatus::NORMAL:
        case PokemonStatus::CONFUSED: return "OK";
        case PokemonStatus::PARALYZED: return "PRZ";
        case PokemonStatus::POISONED: return "PSN";
        case PokemonStatus::FROZEN: return "FRZ";
        case PokemonStatus::ASLEEP: return "SLP";
        case PokemonStatus::BURNED: return "BRN";
        default: return "";
    }
}

bool HaveAllPokemonInRosterFainted
(
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
)
{
    for (const auto& pokemonEntry: pokemonRoster)
    {
        if (pokemonEntry->mHp > 0)
        {
            return false;
        }
    }
    
    return true;
}


size_t GetReadyToEvolvePokemonRosterIndex
(
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
)
{
    for (auto i = 0U; i < pokemonRoster.size(); ++i)
    {
        if (pokemonRoster[i]->mEvolution != nullptr)
        {
            return i;
        }
    }

    return pokemonRoster.size();
}

size_t GetFirstNonFaintedPokemonIndex
(
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
)
{
    for (auto i = 0U; i < pokemonRoster.size(); ++i)
    {
        if (pokemonRoster[i]->mHp > 0)
        {
            return i;
        }
    }
    
    return pokemonRoster.size();
}

size_t GetNumberOfNonFaintedPokemonInParty
(
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
)
{
    auto count = 0U;
    for (const auto& pokemon: pokemonRoster)
    {
        if (pokemon->mHp > 0)
        {
            count++;
        }
    }
    
    return count;
}

size_t GetPokemonRosterIndex
(
    const Pokemon& pokemon,
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
)
{
    for (auto i = 0U; i < pokemonRoster.size(); ++i)
    {
        if (&(pokemon) == pokemonRoster[i].get())
        {
            return i;
        }
    }

    return pokemonRoster.size();
}

CatchResult CalculateCatchResult
(
    const StringId ballNameUsed,
    const Pokemon& pokemon
)
{
    // https://bulbapedia.bulbagarden.net/wiki/Catch_rate

    if (ballNameUsed == StringId("MASTER_BALL"))
    {
        return CatchResult(3, true);
    }

    auto rngN = math::RandomInt(0, 255);

    if (ballNameUsed == StringId("GREAT_BALL"))
    {
        rngN = math::RandomInt(0, 200);
    }
    else if (ballNameUsed == StringId("ULTRA_BALL"))
    {
        rngN = math::RandomInt(0, 150);
    }
    else if (ballNameUsed == StringId("SAFARI_BALL"))
    {
        rngN = math::RandomInt(0, 150);
    }

    auto statusVar = 0;
    if (pokemon.mStatus == PokemonStatus::ASLEEP || pokemon.mStatus == PokemonStatus::FROZEN)
    {
        statusVar = 25;
    }
    else if (pokemon.mStatus == PokemonStatus::PARALYZED || pokemon.mStatus == PokemonStatus::BURNED || pokemon.mStatus == PokemonStatus::POISONED)
    {
        statusVar = 12;
    }

    const auto rVal = rngN - statusVar;
    if (rVal < 0)
    {
        return CatchResult(3, true);
    }
    
    auto fVal = pokemon.mMaxHp * 255;
    fVal /= (ballNameUsed == StringId("GREAT_BALL") ? 8 : 12);

    const auto pokemonHpDivBy4 = pokemon.mHp / 4;
    if (pokemonHpDivBy4 > 0)
    {
        fVal /= pokemonHpDivBy4;
    }

    fVal = math::Min(fVal, 255);
    
    if (pokemon.mBaseSpeciesStats.mCatchRate < rVal)
    {
        return CatchResult(CalculateBallShakeCountBeforePokemonBreaksFree(ballNameUsed, pokemon, fVal), false);
    }

    const auto rngM = math::RandomInt(0, 255);
    if (rngM <= fVal)
    {
        return CatchResult(3, true);
    }
    else
    {
        return CatchResult(CalculateBallShakeCountBeforePokemonBreaksFree(ballNameUsed, pokemon, fVal), false);
    }    
}

int CalculateXpGainFromBattle
(
    const bool isWildBattle,
    const int faintedPokemonXpStat,
    const int faintedPokemonLevel,
    const int numberOfPokemonThatDidNotFaintDuringBattle
)
{
    // https://bulbapedia.bulbagarden.net/wiki/Experience#Experience_gain_in_battle
    return static_cast<int>(((isWildBattle ? 1.0f : 1.5f) * faintedPokemonXpStat * faintedPokemonLevel) / (7.0f * numberOfPokemonThatDidNotFaintDuringBattle));
}

int CalculateHpStat
(
    const int pokemonLevel,
    const int pokemonHpBaseStat,
    const int pokemonHpIv,
    const int pokemonHpEv
)
{
    // https://bulbapedia.bulbagarden.net/wiki/Statistic#cite_note-2
    const auto sqrtTerm = static_cast<int>(math::Sqrt(static_cast<float>(pokemonHpEv))/4.0f);
    const auto divTerm = static_cast<int>((((pokemonHpBaseStat + pokemonHpIv) * 2 + sqrtTerm) * pokemonLevel)/100.0f);
    return divTerm + pokemonLevel + 10;
}

int CalculateStatOtherThanHp
(
    const int pokemonLevel,
    const int pokemonBaseStat,
    const int pokemonStatIv,
    const int pokemonStatEv
)
{
    // https://bulbapedia.bulbagarden.net/wiki/Statistic#cite_note-2
    const auto sqrtTerm = static_cast<int>(math::Sqrt(static_cast<float>(pokemonStatEv)) / 4.0f);
    const auto divTerm = static_cast<int>((((pokemonBaseStat + pokemonStatIv) * 2 + sqrtTerm) * pokemonLevel) / 100.0f);
    return divTerm + 5;
}

int GetStatWithModifierApplied
(
    const int stat,
    const int statModifier
)
{
    return static_cast<int>(stat * (statModifier > 0 ? (statModifier + 3) / 3.0f : (3.0f / (-statModifier + 3))));
}

bool AddToStatStage
(
    const int value,
    int& statStage
)
{
    statStage += value;

    if (statStage > 6)
    {
        statStage = 6;
        return false;
    }
    else if (statStage < -6)
    {
        statStage = -6;
        return false;
    }

    return true;
}

void ResetPokemonEncounterModifierStages
(
    Pokemon& pokemon
) 
{
    pokemon.mAttackEncounterStage  = 0;
    pokemon.mDefenseEncounterStage = 0;
    pokemon.mSpeedEncounterStage   = 0;
    pokemon.mSpecialEncounterStage = 0;
    pokemon.mAccuracyStage         = 0;
    pokemon.mEvasionStage          = 0;
}

void AddToEvStat
(
    const int respectiveDefeatedPokemonBaseStat,
    int& currentEvStat
)
{
    currentEvStat = math::Min(currentEvStat + respectiveDefeatedPokemonBaseStat, 65535);
}

void RestorePokemonStats
(
    Pokemon& pokemon
)
{
    pokemon.mHp     = pokemon.mMaxHp;
    pokemon.mStatus = PokemonStatus::NORMAL;
    
    for (auto i = 0U; i < pokemon.mMoveSet.size(); ++i)
    {
        if (pokemon.mMoveSet[i] != nullptr)
        {
            pokemon.mMoveSet[i]->mPowerPointsLeft = pokemon.mMoveSet[i]->mTotalPowerPoints;
        }        
    }
    
    ResetPokemonEncounterModifierStages(pokemon);
}

void LevelUpStats
(
    const ecs::World& world,
    Pokemon& pokemon
)
{   
    auto targetNextLevel = pokemon.mLevel;
    auto nextLevelXp     = 0;

    do
    {        
        nextLevelXp = CalculatePokemonTotalExperienceAtLevel(pokemon.mBaseSpeciesStats.mSpeciesName, ++targetNextLevel, world);
    } while (nextLevelXp < pokemon.mXpPoints);
    
    targetNextLevel--;
    
    // Set new level
    pokemon.mLevel = targetNextLevel;

    // Calculate new hp stat
    const auto previousMaxHp = pokemon.mMaxHp;
    pokemon.mMaxHp = CalculateHpStat(pokemon.mLevel, pokemon.mBaseSpeciesStats.mHp, pokemon.mHpIv, pokemon.mHpEv);
    pokemon.mHp   += pokemon.mMaxHp - previousMaxHp;

    // Calculate other stats
    pokemon.mAttack  = CalculateStatOtherThanHp(pokemon.mLevel, pokemon.mBaseSpeciesStats.mAttack, pokemon.mAttackIv, pokemon.mAttackEv);
    pokemon.mDefense = CalculateStatOtherThanHp(pokemon.mLevel, pokemon.mBaseSpeciesStats.mDefense, pokemon.mDefenseIv, pokemon.mDefenseEv);
    pokemon.mSpeed   = CalculateStatOtherThanHp(pokemon.mLevel, pokemon.mBaseSpeciesStats.mSpeed, pokemon.mSpeedIv, pokemon.mSpeedEv);
    pokemon.mSpecial = CalculateStatOtherThanHp(pokemon.mLevel, pokemon.mBaseSpeciesStats.mSpecial, pokemon.mSpecialIv, pokemon.mSpecialEv);   

    for (const auto& evolutionInfo : pokemon.mBaseSpeciesStats.mEvolutions)
    {
        if (std::isdigit(evolutionInfo.mEvolutionMethod.GetString().at(0)))
        {
            if (pokemon.mLevel >= std::stoi(evolutionInfo.mEvolutionMethod.GetString()))
            {
                pokemon.mEvolution = CreatePokemon(evolutionInfo.mEvolutionTargetPokemonName, false, pokemon.mLevel, world, &pokemon);
            }            
        }
    }
}

int CalculatePokemonTotalExperienceAtLevel
(
    const StringId pokemonName,
    const int pokemonLevel,
    const ecs::World& world
)
{
    const auto& pokemonBaseStatsComponent = world.GetSingletonComponent<PokemonBaseStatsSingletonComponent>();
    const auto pokemonXpGroup             = pokemonBaseStatsComponent.mPokemonXpGroups.at(pokemonName);

    switch (pokemonXpGroup)
    {
        case XpGroup::SLOW: return static_cast<int>((5.0f * (pokemonLevel * pokemonLevel * pokemonLevel)) / 4.0f);
        case XpGroup::MEDIUM_SLOW: return static_cast<int>(1.2f * (pokemonLevel * pokemonLevel * pokemonLevel) - 15 * (pokemonLevel * pokemonLevel) + 100 * pokemonLevel - 140);
        case XpGroup::MEDIUM_FAST: return pokemonLevel * pokemonLevel * pokemonLevel; 
        case XpGroup::FAST: return static_cast<int>((4.0f * (pokemonLevel * pokemonLevel * pokemonLevel)) / 5.0f);
    }

    assert(false && "Pokemon not present in xp groups");
    return 0;
}

const PokemonBaseStats& GetPokemonBaseStats
(
    const StringId pokemonName,
    const ecs::World& world
)
{
    const auto& pokemonBaseStatsComponent = world.GetSingletonComponent<PokemonBaseStatsSingletonComponent>();
    return pokemonBaseStatsComponent.mPokemonBaseStats.at(pokemonName);
}

PokedexEntryType GetPokedexEntryTypeForPokemon
(
    const StringId pokemonName,
    const ecs::World& world
)
{
    const auto& baseStats             = GetPokemonBaseStats(pokemonName, world);
    const auto& pokedexStateComponent = world.GetSingletonComponent<PokedexStateSingletonComponent>();

    return pokedexStateComponent.mPokedexEntries[baseStats.mId - 1];
}

bool DoesPokemonHaveType
(
    const StringId type,
    const Pokemon& pokemon
)
{
    return pokemon.mBaseSpeciesStats.mFirstType == type || pokemon.mBaseSpeciesStats.mSecondType == type;
}

bool DoesPokemonHaveType
(
    const PokemonType type,
    const Pokemon& pokemon
)
{
    return sPokemonTypesToStrings.at(type) == pokemon.mBaseSpeciesStats.mFirstType || sPokemonTypesToStrings.at(type) == pokemon.mBaseSpeciesStats.mSecondType;
}

void LoadAndPopulatePokemonBaseStats
(
    PokemonBaseStatsSingletonComponent& pokemonBaseStatsComponent
)
{
    static const std::unordered_map<StringId, OverworldPokemonSpriteType, StringIdHasher> overworldSpriteNamesToEnumValues =
    {
        { StringId("BALL"),    OverworldPokemonSpriteType::BALL },
        { StringId("BEAST"),   OverworldPokemonSpriteType::BEAST },
        { StringId("BUG"),     OverworldPokemonSpriteType::BUG },
        { StringId("GRASS"),   OverworldPokemonSpriteType::GRASS },
        { StringId("DRAGON"),  OverworldPokemonSpriteType::DRAGON },
        { StringId("FLYING"),  OverworldPokemonSpriteType::FLYING },
        { StringId("FOSSIL"),  OverworldPokemonSpriteType::FOSSIL },
        { StringId("NORMAL"),  OverworldPokemonSpriteType::NORMAL },
        { StringId("PIKACHU"), OverworldPokemonSpriteType::PIKACHU },
        { StringId("WATER"),   OverworldPokemonSpriteType::WATER },
    };

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    
    // Load pokemon Xp groups
    const auto pokemonXpGroupsFilePath = ResourceLoadingService::RES_DATA_ROOT + POKEMON_XP_GROUPS_FILE_NAME;
    resourceLoadingService.LoadResource(pokemonXpGroupsFilePath);
    const auto& pokemonXpGroupsFileResource = resourceLoadingService.GetResource<DataFileResource>(pokemonXpGroupsFilePath);

    // Parse xp groups json
    const auto xpGroupsJson = nlohmann::json::parse(pokemonXpGroupsFileResource.GetContents());

    for (const auto& slowGroupEntry : xpGroupsJson["SLOW"])        pokemonBaseStatsComponent.mPokemonXpGroups[StringId(slowGroupEntry.get<std::string>())] = XpGroup::SLOW;
    for (const auto& slowGroupEntry : xpGroupsJson["MEDIUM_SLOW"]) pokemonBaseStatsComponent.mPokemonXpGroups[StringId(slowGroupEntry.get<std::string>())] = XpGroup::MEDIUM_SLOW;
    for (const auto& slowGroupEntry : xpGroupsJson["MEDIUM_FAST"]) pokemonBaseStatsComponent.mPokemonXpGroups[StringId(slowGroupEntry.get<std::string>())] = XpGroup::MEDIUM_FAST;
    for (const auto& slowGroupEntry : xpGroupsJson["FAST"])        pokemonBaseStatsComponent.mPokemonXpGroups[StringId(slowGroupEntry.get<std::string>())] = XpGroup::FAST;

    // Get pokemon base stats data file resource
    const auto pokemonBaseStatsFilePath = ResourceLoadingService::RES_DATA_ROOT + BASE_STATS_FILE_NAME;
    resourceLoadingService.LoadResource(pokemonBaseStatsFilePath);
    const auto& baseStatsFileResource = resourceLoadingService.GetResource<DataFileResource>(pokemonBaseStatsFilePath);

    // Parse base stats json
    const auto baseStatsJson = nlohmann::json::parse(baseStatsFileResource.GetContents());
    
    auto& pokemonBaseStats = pokemonBaseStatsComponent.mPokemonBaseStats;
    for (auto it = baseStatsJson.begin(); it != baseStatsJson.end(); ++it)
    {
        const auto pokemonName  = StringId(it.key());
        const auto& statsObject = it.value();

        const auto id              = statsObject["id"].get<int>();
        const auto hp              = statsObject["hp"].get<int>();
        const auto attack          = statsObject["attack"].get<int>();
        const auto defense         = statsObject["defense"].get<int>();
        const auto speed           = statsObject["speed"].get<int>();
        const auto special         = statsObject["special"].get<int>();
        const auto xpStat          = statsObject["xpstat"].get<int>();
        const auto catchRate       = statsObject["catchrate"].get<int>();
        const auto overworldSprite = overworldSpriteNamesToEnumValues.at(statsObject["ovimagetype"].get<std::string>());

        const auto& typeStringsArray = statsObject["type"];
        auto firstType  = StringId();
        auto secondType = StringId();

        firstType = StringId(typeStringsArray[0]["name"].get<std::string>());

        if (typeStringsArray.size() == 2)
        {
            secondType = StringId(typeStringsArray[1]["name"].get<std::string>());
        }
        
        std::vector<EvolutionInfo> evolutions;
        for (const auto& evolutionInfo : statsObject["evolution"])
        {
            evolutions.push_back
            ({
                StringId(evolutionInfo["name"].get<std::string>()),
                StringId(evolutionInfo["method"].get<std::string>())
            });
        }

        std::vector<MoveLearnInfo> learnset;
        for (const auto& moveLearnInfo : statsObject["moveset"])
        {
            learnset.push_back
            ({
                StringId(moveLearnInfo["movename"].get<std::string>()),
                moveLearnInfo["level"].get<int>()
            });
        }

        pokemonBaseStats.insert(std::make_pair(pokemonName, PokemonBaseStats
        (
            evolutions,
            learnset,   
            statsObject["text"].get<std::string>(),
            statsObject["dextype"].get<std::string>(),
            statsObject["body"].get<std::string>(),
            pokemonName,
            firstType, 
            secondType, 
            id,
            hp,
            attack, 
            defense, 
            speed, 
            special,
            xpStat, 
            catchRate, 
            overworldSprite
        )));
    }
}

void ChangePokedexEntryForPokemon
(
    const StringId pokemonName,
    const PokedexEntryType pokedexEntryType,
    const ecs::World& world
)
{
    const auto& baseStats       = GetPokemonBaseStats(pokemonName, world);
    auto& pokedexStateComponent = world.GetSingletonComponent<PokedexStateSingletonComponent>();
    pokedexStateComponent.mPokedexEntries[baseStats.mId - 1] = pokedexEntryType;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

int CalculateBallShakeCountBeforePokemonBreaksFree
(
    const StringId ballNameUsed,
    const Pokemon& pokemon,
    const int fVal
)
{
    // https://bulbapedia.bulbagarden.net/wiki/Catch_rate
    auto dVal = static_cast<int>(pokemon.mBaseSpeciesStats.mCatchRate * 100 / 255.0f);

    if (ballNameUsed == StringId("GREAT_BALL"))
    {
        dVal = static_cast<int>(pokemon.mBaseSpeciesStats.mCatchRate * 100 / 200.0f);
    }
    else if (ballNameUsed == StringId("ULTRA_BALL"))
    {
        dVal = static_cast<int>(pokemon.mBaseSpeciesStats.mCatchRate * 100 / 150.0f);
    }
    else if (ballNameUsed == StringId("SAFARI_BALL"))
    {
        dVal = static_cast<int>(pokemon.mBaseSpeciesStats.mCatchRate * 100 / 150.0f);
    }

    if (dVal >= 256)
    {
        return 3;
    }
    else
    {
        auto xVal = static_cast<int>(dVal * fVal / 255.0f);
        if (pokemon.mStatus == PokemonStatus::ASLEEP || pokemon.mStatus == PokemonStatus::FROZEN)
        {
            xVal += 10;
        }
        else if (pokemon.mStatus == PokemonStatus::PARALYZED || pokemon.mStatus == PokemonStatus::BURNED || pokemon.mStatus == PokemonStatus::POISONED)
        {
            xVal += 5;
        }

        if (xVal < 10)
        {
            return 0;
        }
        else if (xVal < 30)
        {
            return 1;
        }
        else if (xVal < 70)
        {
            return 2;
        }
        else
        {
            return 3;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

