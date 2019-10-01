//
//  PokemonUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/07/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokemonUtils_h
#define PokemonUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "StringUtils.h"
#include "../GameConstants.h"
#include "../../ECS.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct Pokemon;
struct PokemonMoveStats;
struct PokemonBaseStats;
class PokemonBaseStatsSingletonComponent;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct CatchResult
{    
    CatchResult
    (
        const int numberOfBallShakes,
        const bool wasCaught
    )
        : mNumberOfBallShakes(numberOfBallShakes)
        , mWasCaught(wasCaught)
    {
    }

    const int mNumberOfBallShakes;        
    const bool mWasCaught;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Pokemon> CreatePokemon
(
    const StringId pokemonName,
    const int pokemonLevel,
    const bool trainerOwned,
    const ecs::World& world,
    const Pokemon* const priorEvolution = nullptr
);

std::string GetFormattedPokemonIdString
(
    const int pokemonId
);

std::string GetFormattedPokemonStatus
(
    const int pokemonHp,
    const PokemonStatus pokemonStatus
);

bool HaveAllPokemonInRosterFainted
(
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
);

size_t GetReadyToEvolvePokemonRosterIndex
(
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
);


size_t GetFirstNonFaintedPokemonIndex
(
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
);

size_t GetNumberOfNonFaintedPokemonInParty
(
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
);

size_t GetPokemonRosterIndex
(
    const Pokemon&,
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
);

CatchResult CalculateCatchResult
(
    const StringId ballNameUsed,
    const Pokemon& pokemon
);

int CalculateXpGainFromBattle
(
    const bool isWildBattle,
    const int faintedPokemonXpStat,
    const int faintedPokemonLevel,
    const int numberOfPokemonThatDidNotFaintDuringBattle  
);

int CalculateHpStat
(
    const int pokemonLevel,
    const int pokemonHpBaseStat,
    const int pokemonHpIv,
    const int pokemonHpEv
);

int CalculateStatOtherThanHp
(
    const int pokemonLevel,
    const int pokemonBaseStat,
    const int pokemonStatIv,
    const int pokemonStatEv
);

int GetStatWithModifierApplied
(
    const int stat,
    const int statModifier
);

// Returns true if the value changed 
// post addition or false otherwise
bool AddToStatStage
(
    const int value,
    int& statStage
);

void ResetPokemonEncounterModifierStages
(
    Pokemon& pokemon
);

void AddToEvStat
(
    const int respectiveDefeatedPokemonBaseStat,
    int& currentEvStat
);

void RestorePokemonStats
(
    Pokemon& pokemon
);

void LevelUpStats
(
    const ecs::World& world,
    Pokemon& pokemon
);

int CalculatePokemonTotalExperienceAtLevel
(
    const StringId pokemonName,
    const int pokemonLevel,
    const ecs::World& world
);

const PokemonBaseStats& GetPokemonBaseStats
(
    const StringId pokemonName,
    const ecs::World& world
);

bool DoesPokemonHaveType
(
    const StringId type,
    const Pokemon& pokemon
);

bool DoesPokemonHaveType
(
    const PokemonType type,
    const Pokemon& pokemon
);

void LoadAndPopulatePokemonBaseStats
(
    PokemonBaseStatsSingletonComponent& pokemonBaseStatsComponent
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonUtils_h */
