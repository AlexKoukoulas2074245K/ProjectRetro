//
//  SeaStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 15/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef SeaStateSingletonComponent_h
#define SeaStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/utils/Timer.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class SeaStateSingletonComponent final: public ecs::IComponent
{   
public:
    std::unique_ptr<Timer> mSeaStateTimer = nullptr;
    int mMovementCounter                  = 0;
    bool mWaveDirectionLeft               = true;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* SeaStateSingletonComponent_h */
