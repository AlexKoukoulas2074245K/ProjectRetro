//
//  PokemonMoveUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/07/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokemonMoveUtils_h
#define PokemonMoveUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../GameConstants.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct Pokemon;
struct PokemonMoveStats;
class MoveStatsSingletonComponent;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const StringId POISON_TICK_MOVE_NAME           = StringId("POISON_TICK");
static const StringId CONFUSION_HURT_ITSELF_MOVE_NAME = StringId("CONFUSION_HURT_ITSELF");

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

size_t FindFirstUnusedMoveIndex(const PokemonMoveSet& moveset);

void LoadAndPopulateMoveStats(MoveStatsSingletonComponent&);

float GetTypeEffectiveness
(
    const StringId attackingMoveType,
    const StringId defendingMoveType,
    const ecs::World& world
);

const PokemonMoveStats& GetMoveStats
(
    const StringId moveName,
    const ecs::World& world
);

bool isMoveSpecial
(
    const StringId moveType   
);

bool isMoveNonShake
(
    const StringId moveName
);

bool DoesMovesetHaveMove
(
    const StringId moveName,
    const PokemonMoveSet& moveset
);

bool ShouldAttackingPokemonBeFullyParalyzed
(
    const Pokemon& pokemon
);

bool ShouldMoveMiss
(
    const int moveAccuracy,
    const int attackerAccuracyStage,
    const int defenderEvasionStage
);

bool ShouldMoveCrit
(
    const StringId moveName,    
    const int attackerBaseSpeed
);

bool ShouldOpponentGoFirst
(
    const StringId playersMoveName,
    const StringId opponentsMoveName,
    const int playersPokemonSpeedModifier,
    const int opponentsPokemonSpeedModifier,
    const int playersPokemonSpeed,
    const int opponentsPokemonSpeed,
    const PokemonStatus playerPokemonStatus,
    const PokemonStatus opponentPokemonStatus
);

bool DoesMoveHaveSpeciallyHandledAnimation
(
    const StringId moveName
);

int CalculateDamage
(
    const int attackingPokemonLevel,
    const int attackingMovePower,
    const int attackingStatModifier,
    const int defensiveStatModifier,
    const int attackingPokemonAttackingStat, // Attack or Special
    const int defendingPokemonDefensiveStat, // Defense or Special
    const float effectivenessFactor,
    const bool isCrit,
    const bool isStab
);

int CalculatePokemonHurtingItselfDamage
(
    const int pokemonLevel,
    const int pokemonAttackingStat,
    const int pokemonDefendingStat
);

void AddMoveToIndex
(
	const StringId moveName,
	const size_t moveIndex,
	const ecs::World& world,
	Pokemon& pokemon
);

void AddMoveToFirstUnusedIndex
(
	const StringId moveName,
	const ecs::World& world,
	Pokemon& pokemon
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonMoveUtils_h */
