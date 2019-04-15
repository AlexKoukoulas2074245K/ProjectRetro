//
//  RenderingUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef RenderingUtils_h
#define RenderingUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../common/GameConstants.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

// small optimization 
static glm::vec4 planes[6];

bool IsMeshInsideCameraFrustum
(
    const glm::vec3& meshPosition,
    const glm::vec3& meshScale,
    const glm::vec3& meshDimensions, 
    const glm::mat4& viewMatrix, 
    const glm::mat4& projectionMatrix
)
{
    auto viewProjectionMatrix = projectionMatrix * viewMatrix;

    const auto rowX = glm::row(viewProjectionMatrix, 0);
    const auto rowY = glm::row(viewProjectionMatrix, 1);
    const auto rowZ = glm::row(viewProjectionMatrix, 2);
    const auto rowW = glm::row(viewProjectionMatrix, 3);
    
    planes[0] = glm::normalize(rowW + rowX);
    planes[1] = glm::normalize(rowW - rowX);
    planes[2] = glm::normalize(rowW + rowY);
    planes[3] = glm::normalize(rowW - rowY);
    planes[4] = glm::normalize(rowW + rowZ);
    planes[5] = glm::normalize(rowW - rowZ);

    for (auto i = 0U; i < 6U; ++i)
    {
        glm::vec3 planeNormal(planes[i].x, planes[i].y, planes[i].z);
        const auto length = glm::length(planeNormal);
        planes[i] = -planes[i] / length;
    }

    const auto scaledMeshDimensions = meshDimensions * meshScale;
    const auto frustumCheckSphereRadius = math::Max(scaledMeshDimensions.x, math::Max(scaledMeshDimensions.y, scaledMeshDimensions.z));

    for (auto i = 0U; i < 6U; ++i)
    {
        float dist = 
            planes[i].x * meshPosition.x + 
            planes[i].y * meshPosition.y + 
            planes[i].z * meshPosition.z + 
            planes[i].w - frustumCheckSphereRadius;

        if (dist > 0) return false;
    }
  
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* RenderingUtils_h */
