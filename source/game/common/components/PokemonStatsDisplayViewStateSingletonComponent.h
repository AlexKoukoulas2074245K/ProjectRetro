#//
//  PokemonStatsDisplayViewStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 23/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokemonStatsDisplayViewStateSingletonComponent_h
#define PokemonStatsDisplayViewStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class PokemonStatsDisplayViewCreationSourceType
{
    POKEMON_SELECTION_VIEW, PC
};

class PokemonStatsDisplayViewStateSingletonComponent final: public ecs::IComponent
{
public:    
    ecs::EntityId mPokemonStatsInvisibleTextboxEntityId          = ecs::NULL_ENTITY_ID;
    ecs::EntityId mBackgroundCoverEntityId                       = ecs::NULL_ENTITY_ID;
    ecs::EntityId mStatsLayoutsEntityId                          = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPokemonFrontSpriteEntityId                    = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPokemonHealthbarEntityId                      = ecs::NULL_ENTITY_ID;
    PokemonStatsDisplayViewCreationSourceType mSourceCreatorFlow = PokemonStatsDisplayViewCreationSourceType::POKEMON_SELECTION_VIEW;
    bool mIsInScreen1                                            = false;    

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonStatsDisplayViewStateSingletonComponent_h */
