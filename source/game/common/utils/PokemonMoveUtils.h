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

struct PokemonMoveStats;
class MoveStatsSingletonComponent;

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

bool ShouldMoveMiss
(
    const int moveAccuracy,
    const int attackerAccuracyStage,
    const int defenderEvasionStage
);

bool ShouldMoveCrit
(
    const StringId moveName,
    const int attackerSpeed
);

bool ShouldOpponentGoFirst
(
    const StringId playersMoveName,
    const StringId opponentsMoveName,
    const int playersPokemonSpeed,
    const int opponentsPokemonSpeed
);

int CalculateDamage
(
    const int attackingPokemonLevel,
    const int attackingMovePower,
    const int attackingPokemonAttackingStat, // Attack or Special
    const int defendingPokemonDefensiveStat, // Defense or Special
    const float effectivenessFactor,
    const bool isCrit,
    const bool isStab
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonMoveUtils_h */
