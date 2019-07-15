//
//  EncounterShakeSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 23/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef EncounterShakeSingletonComponent_h
#define EncounterShakeSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/utils/Timer.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class ShakeType
{
    NONE,
    OPPONENT_POKEMON_BLINK,
    OPPONENT_POKEMON_SHORT_HORIZONTAL_SHAKE,
    OPPONENT_POKEMON_LONG_HORIZONTAL_SHAKE,
    PLAYER_POKEMON_VERTICAL_SHAKE,
    PLAYER_POKEMON_LONG_HORIZONTAL_SHAKE,
    PLAYER_RAPID_LONG_HORIZONTAL_SHAKE
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float SHAKE_TIME_DELAY    = 0.05f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class EncounterShakeSingletonComponent final: public ecs::IComponent
{
public:
    std::unique_ptr<Timer> mShakeTimeDelayTimer = nullptr;
    int mShakeProgressionStep                   = 0;
    ShakeType mActiveShakeType                  = ShakeType::NONE;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* EncounterShakeSingletonComponent_h */
