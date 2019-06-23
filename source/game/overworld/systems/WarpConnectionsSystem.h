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
#include "../../common/utils/StringUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class WarpConnectionsSystem final: public ecs::BaseSystem
{
public:
    WarpConnectionsSystem(ecs::World&);
    
    void VUpdateAssociatedComponents(const float dt) const override;
    
private:
    void PopulateWarpConnections() const;

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* WarpConnectionsSystem_h */
