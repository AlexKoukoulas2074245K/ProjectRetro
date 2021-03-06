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
#include "../../common/utils/Timer.h"

#include <array>
#include <memory>
#include <vector>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class PokemonSelectionViewCreationSourceType
{
    ENCOUNTER_AFTER_POKEMON_FAINTED,
    ENCOUNTER_FROM_MAIN_MENU,
    OVERWORLD,
    ITEM_USAGE
};

enum class PokemonSelectionViewOperationState
{
    INVALID_OPERATION,
    INDEX_SWAP_FLOW,
    HEALING_UP,
    NORMAL
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokemonSelectionViewStateSingletonComponent final: public ecs::IComponent
{
public:
    std::vector<std::array<ecs::EntityId, 3>> mPokemonSpriteEntityIds;
    std::unique_ptr<Timer> mSwapIndexAnimationTimer            = nullptr;
    ecs::EntityId mBackgroundEntityId                          = ecs::NULL_ENTITY_ID;    
    int mLastSelectedPokemonRosterIndex                        = 0;
    int mIndexSwapOriginPokemonCursorIndex                     = 0;
    int mSwapAnimationStep                                     = 0;
    float mFloatPokemonHealth                                  = 0.0f;
    float mHealthToRestoreCapacity                             = 0.0f;
    PokemonSelectionViewCreationSourceType mCreationSourceType = PokemonSelectionViewCreationSourceType::OVERWORLD;
    PokemonSelectionViewOperationState mOperationState         = PokemonSelectionViewOperationState::NORMAL;
    bool mPokemonHasBeenSelected                               = false;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonSelectionViewEntitiesSingletonComponent_h */
