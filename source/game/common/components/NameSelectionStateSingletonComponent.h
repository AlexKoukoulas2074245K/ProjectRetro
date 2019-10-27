//
//  NameSelectionStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef NameSelectionStateSingletonComponent_h
#define NameSelectionStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../utils/StringUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class NameSelectionMode
{
    PLAYER_NAME, RIVAL_NAME, POKEMON_NICKNAME
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct Pokemon;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class NameSelectionStateSingletonComponent final: public ecs::IComponent
{
public:
    NameSelectionMode mNameSelectionMode = NameSelectionMode::PLAYER_NAME;
    Pokemon* mPokemonToSelectNameFor = nullptr;
    std::string mSelectedName = "";
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* NameSelectionStateSingletonComponent_h */
