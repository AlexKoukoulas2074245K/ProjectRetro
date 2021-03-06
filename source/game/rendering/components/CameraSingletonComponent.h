//
//  CameraSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef CameraSingletonComponent_h
#define CameraSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/utils/MathUtils.h"

#include <array>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const unsigned int CAMERA_FRUSTUM_SIDES = 6;
const float CAMERA_FIELD_OF_VIEW        = math::PI/7.0f;
const float CAMERA_Z_NEAR               = 13.0f;
const float CAMERA_Z_FAR                = 40.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

using CameraFrustum = std::array<glm::vec4, CAMERA_FRUSTUM_SIDES>;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class CameraSingletonComponent final: public ecs::IComponent
{
public:
    CameraFrustum mFrustum;
    glm::mat4 mViewMatrix         = glm::mat4(1.0f);
    glm::mat4 mProjectionMatrix   = glm::mat4(1.0f);
    glm::vec3 mPosition           = glm::vec3(0.0f, 18.4f, 0.0f);
    glm::vec3 mFocusPosition      = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mUpVector           = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 mGlobalScreenOffset = glm::vec3(0.0f, 0.0f, 0.0f);
    float mFieldOfView            = CAMERA_FIELD_OF_VIEW;
    float mZNear                  = CAMERA_Z_NEAR;
    float mZFar                   = CAMERA_Z_FAR;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* CameraSingletonComponent_h */
