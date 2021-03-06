//
//  PokeCenterHealingAnimationStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokeCenterHealingAnimationStateSingletonComponent_h
#define PokeCenterHealingAnimationStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/utils/Timer.h"

#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class PokeCenterHealingAnimationState
{
    ANIMATION_START,
    JOY_FACING_NORTH,
    JOY_FACING_WEST,
    POKEBALL_PLACEMENT,
    COLOR_SWAPPING,
    HEALING_FINISHED_DELAY,
    HEALING_FINISHED_JOY_FACING_NORTH,
    THANK_YOU_DIALOG,
    JOY_BOW
};

static const std::unordered_map<PokeCenterHealingAnimationState, float> sHealingAnimationStateDurations = 
{
    { PokeCenterHealingAnimationState::ANIMATION_START,                   1.0f },
    { PokeCenterHealingAnimationState::JOY_FACING_NORTH,                  1.0f },
    { PokeCenterHealingAnimationState::JOY_FACING_WEST,                   1.0f },
    { PokeCenterHealingAnimationState::POKEBALL_PLACEMENT,                0.6f },
    { PokeCenterHealingAnimationState::COLOR_SWAPPING,                    0.25f },
    { PokeCenterHealingAnimationState::HEALING_FINISHED_DELAY,            0.5f },
    { PokeCenterHealingAnimationState::HEALING_FINISHED_JOY_FACING_NORTH, 1.0f },
    { PokeCenterHealingAnimationState::THANK_YOU_DIALOG,                  1.0f },
    { PokeCenterHealingAnimationState::JOY_BOW,                           1.0f },
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokeCenterHealingAnimationStateSingletonComponent final: public ecs::IComponent
{   
public:
    std::queue<PokeCenterHealingAnimationState> mHealingAnimationStateQueue;
    std::vector<ecs::EntityId> mPokeballEntityIds;    
    std::unique_ptr<Timer> mAnimationTimer = nullptr;    
    int mFlashingCounter                   = 0;
    bool mInvertedColors                   = false;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokeCenterHealingAnimationStateSingletonComponent_h */
