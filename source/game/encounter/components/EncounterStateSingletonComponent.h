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
#include "../flowstates/BaseEncounterFlowState.h"

#include <memory>

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

class EncounterStateSingletonComponent final: public ecs::IComponent
{
public:    
    std::unique_ptr<BaseEncounterFlowState> mActiveEncounterFlowState = nullptr;
    EncounterType mActiveEncounterType = EncounterType::NONE;
    OverworldEncounterAnimationState mOverworldEncounterAnimationState = OverworldEncounterAnimationState::NONE;    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* EncounterStateSingletonComponent_h */
