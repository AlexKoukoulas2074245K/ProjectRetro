//
//  WarpConnectionsSystem.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef WarpConnectionsSystem_h
#define WarpConnectionsSystem_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class WarpConnectionsSystem final: public ecs::BaseSystem
{
public:
    WarpConnectionsSystem(ecs::World&);
    
    void VUpdateAssociatedComponents(const float dt) const override;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* WarpConnectionsSystem_h */