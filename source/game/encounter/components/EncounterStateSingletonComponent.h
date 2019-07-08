//
//  EncounterStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 23/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef EncounterStateSingletonComponent_h
#define EncounterStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/GameConstants.h"
#include "../../common/flowstates/FlowStateManager.h"
#include "../../common/utils/Timer.h"
#include "../../resources/ResourceLoadingService.h"

#include <memory>
#include <queue>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class EncounterType
{
    NONE, WILD, TRAINER
};

enum class OverworldEncounterAnimationState
{
    NONE, SCREEN_FLASH, ENCOUNTER_INTRO_ANIMATION, ENCOUNTER_INTRO_ANIMATION_COMPLETE
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const float BATTLE_ANIMATION_FRAME_DURATION = 0.04f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class EncounterViewObjects final
{    
public:
    std::queue<ResourceId> mBattleAnimationFrameResourceIdQueue;
    std::unique_ptr<Timer> mBattleAnimationTimer;
    ecs::EntityId mBattleAnimationFrameEntityId       = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPlayerActiveSpriteEntityId         = ecs::NULL_ENTITY_ID;
    ecs::EntityId mOpponentActiveSpriteEntityId       = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPlayerStatusDisplayEntityId        = ecs::NULL_ENTITY_ID;
    ecs::EntityId mOpponentStatusDisplayEntityId      = ecs::NULL_ENTITY_ID;
    ecs::EntityId mOpponentPokemonInfoTextboxEntityId = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPlayerPokemonInfoTextboxEntityId   = ecs::NULL_ENTITY_ID;
    ecs::EntityId mLevelLeftEdgeEntityId              = ecs::NULL_ENTITY_ID;
    ecs::EntityId mLevelRightEdgeEntityId             = ecs::NULL_ENTITY_ID;
    ecs::EntityId mOpponentPokemonHealthBarEntityId   = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPlayerPokemonHealthBarEntityId     = ecs::NULL_ENTITY_ID;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class EncounterStateSingletonComponent final: public ecs::IComponent
{
public:    
    FlowStateManager mFlowStateManager;
    EncounterViewObjects mViewObjects;
    std::vector<PokemonInfo> mOpponentPokemonRoster;        
    EncounterType mActiveEncounterType                                 = EncounterType::NONE;
    OverworldEncounterAnimationState mOverworldEncounterAnimationState = OverworldEncounterAnimationState::NONE;            
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* EncounterStateSingletonComponent_h */
