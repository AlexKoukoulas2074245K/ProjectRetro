//
//  PokedexUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/08/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokedexUtils_h
#define PokedexUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../components/PokedexStateSingletonComponent.h"
#include "../../common/utils/MathUtils.h"
#include "../../ECS.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId LoadAndCreatePokedexPokemonDataScreen
(
    ecs::World& world
);

PokedexEntryType GetPokedexEntryTypeForPokemon
(
    const StringId pokemonName,
    const ecs::World& world
);

std::string GetPokedexCompletionRatingText
(
    const ecs::World& world
);

void ChangePokedexEntryForPokemon
(
    const StringId pokemonName,
    const PokedexEntryType pokedexEntryType,
    const ecs::World& world
);

int GetNumberOfPokemonWithPokedexEntryType
(
    const PokedexEntryType pokedexEntryType,
    const ecs::World& world
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokedexUtils_h */