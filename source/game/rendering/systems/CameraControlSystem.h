//
//  CameraControlSystem.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef CameraControlSystem_h
#define CameraControlSystem_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class CameraControlSystem final : public ecs::BaseSystem
{
public:
    CameraControlSystem(ecs::World&);

    void VUpdateAssociatedComponents(const float dt) const override;

    static const float CAMERA_Z_DISTANCE_FROM_PLAYER;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* CameraControlSystem_h */
