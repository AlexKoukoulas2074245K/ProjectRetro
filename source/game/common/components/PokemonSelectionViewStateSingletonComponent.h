//
//  PokemonSelectionViewEntitiesSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokemonSelectionViewEntitiesSingletonComponent_h
#define PokemonSelectionViewEntitiesSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"

#include <array>
#include <vector>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class  PokemonSelectionViewCreationSourceType
{
    ENCOUNTER_AFTER_POKEMON_FAINTED,
    ENCOUNTER_FROM_MAIN_MENU,
    OVERWORLD
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokemonSelectionViewStateSingletonComponent final: public ecs::IComponent
{
public:
    std::vector<std::array<ecs::EntityId, 3>> mPokemonSpriteEntityIds;
    ecs::EntityId mBackgroundEntityId                          = ecs::NULL_ENTITY_ID;    
    int mLastSelectedPokemonRosterIndex                        = 0;
    PokemonSelectionViewCreationSourceType mCreationSourceType = PokemonSelectionViewCreationSourceType::OVERWORLD;
    bool mPokemonHasBeenSelected                               = false;
    bool mNoWillToFightTextFlowActive                          = false;
    bool mIndexSwapFlowActive                                  = false;

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonSelectionViewEntitiesSingletonComponent_h */
