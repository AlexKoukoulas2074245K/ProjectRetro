//
//  EncounterSpriteUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/06/2019.
//

#ifndef EncounterSpriteUtils_h
#define EncounterSpriteUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/GameConstants.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/StringUtils.h"

#include <utility>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct PokemonInfo;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId LoadAndCreateTrainerSprite
(
    const int atlasCol,
    const int atlasRow,
    const glm::vec3& spritePosition,
    const glm::vec3& spriteScale,
    ecs::World& world
);

ecs::EntityId LoadAndCreatePokemonSprite
(
    const StringId pokemonName,
    const bool frontFace,
    const glm::vec3& spritePosition,
    const glm::vec3& spriteScale,
    ecs::World& world
);

ecs::EntityId LoadAndCreatePlayerRosterDisplay
(
    const glm::vec3& spritePosition,
    const glm::vec3& spriteScale,
    ecs::World& world
);

ecs::EntityId LoadAndCreatePlayerPokemonStatusDisplay
(
    const glm::vec3& spritePosition,
    const glm::vec3& spriteScale,
    ecs::World& world
);

ecs::EntityId LoadAndCreatePokemonSelectionViewBareHealthbarContainer
(
    const size_t pokemonRosterIndex,
    ecs::World& world
);

ecs::EntityId LoadAndCreateOpponentPokemonDeathCover
(
    const glm::vec3& spritePosition,
    const glm::vec3& spriteScale,
    ecs::World& world
);

ecs::EntityId LoadAndCreateOpponentPokemonStatusDisplay
(
    const glm::vec3& spritePosition,
    const glm::vec3& spriteScale,
    ecs::World& world
);

ecs::EntityId LoadAndCreatePokemonHealthBar
(
    const float depletionProportion,
    const bool isOpponentHealthBar,
    ecs::World& world,
    const bool isInPokemonSelectionView = false,
    const size_t pokemonRosterIndex = 0
);

std::pair<ecs::EntityId, ecs::EntityId> LoadAndCreateLevelEdges(ecs::World&);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* EncounterSpriteUtils_h */
