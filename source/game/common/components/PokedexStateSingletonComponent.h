#//
//  PokedexStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokedexStateSingletonComponent_h
#define PokedexStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../GameConstants.h"
#include "../utils/Timer.h"
#include "../../ECS.h"

#include <array>
#include <memory>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class PokedexEntryType
{
    LOCKED, SEEN, OWNED
};

enum class PokedexPageEntryType
{
    LOCKED,
    DETAILS_LOCKED,
    DETAILS_1,
    DETAILS_2
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const float POKEDEX_VIEW_TIMER_DURATION = 1.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokedexStateSingletonComponent final: public ecs::IComponent
{
public:        
    std::array<PokedexEntryType, MAX_POKEMON_ID> mPokedexEntries{ PokedexEntryType::LOCKED };
    StringId mSelectedPokemonName                             = StringId();
    ecs::EntityId mPokemonSpriteEntityId                      = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPokedexBackgroundSpriteEntityId            = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPokedexInfoTextboxEntityId                 = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPokedexMainViewBackgroundEntityId          = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPokedexSelectionOptionsBareTextboxEntityId = ecs::NULL_ENTITY_ID;
    std::unique_ptr<Timer> mPokedexViewTimer                  = nullptr;
    PokedexPageEntryType mCurrentPageViewType                 = PokedexPageEntryType::LOCKED;        
    bool mPokedexPokemonDescriptionCursorShowing              = false;

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokedexStateSingletonComponent_h */
