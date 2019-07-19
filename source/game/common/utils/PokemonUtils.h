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

#include "../../ECS.h"
#include "StringUtils.h"

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

std::unique_ptr<Pokemon> CreatePokemon
(
    const StringId pokemonName,
    const int pokemonLevel,
    const ecs::World& world
);

bool HaveAllPokemonInRosterFainted
(
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
);

size_t GetFirstNonFaintedPokemonIndex
(
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
);

size_t GetPokemonRosterIndex
(
    const Pokemon&,
    const std::vector<std::unique_ptr<Pokemon>>& pokemonRoster
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

void ResetPokemonEncounterModifierStages
(
    Pokemon& pokemon
);

void AddToEvStat
(
    const int respectiveDefeatedPokemonBaseStat,
    int& currentEvStat
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

void LoadAndPopulatePokemonBaseStats
(
    PokemonBaseStatsSingletonComponent& pokemonBaseStatsComponent
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonUtils_h */
