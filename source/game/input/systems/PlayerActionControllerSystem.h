//
//  PlayerActionControllerSystem.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PlayerActionControllerSystem_h
#define PlayerActionControllerSystem_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PlayerActionControllerSystem final: public ecs::BaseSystem
{
public:
    PlayerActionControllerSystem(ecs::World&);
    
    void VUpdate(const float dt) override;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PlayerActionControllerSystem_h */
