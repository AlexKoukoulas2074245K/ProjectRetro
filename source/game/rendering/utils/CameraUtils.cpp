//
//  CameraUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 09/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "CameraUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

CameraFrustum CalculateCameraFrustum(const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
{
    CameraFrustum cameraFrustum;

    auto viewProjectionMatrix = projMatrix * viewMatrix;

    // Extract rows from combined view projection matrix
    const auto rowX = glm::row(viewProjectionMatrix, 0);
    const auto rowY = glm::row(viewProjectionMatrix, 1);
    const auto rowZ = glm::row(viewProjectionMatrix, 2);
    const auto rowW = glm::row(viewProjectionMatrix, 3);

    // Calculate planes
    cameraFrustum[0] = glm::normalize(rowW + rowX);
    cameraFrustum[1] = glm::normalize(rowW - rowX);
    cameraFrustum[2] = glm::normalize(rowW + rowY);
    cameraFrustum[3] = glm::normalize(rowW - rowY);
    cameraFrustum[4] = glm::normalize(rowW + rowZ);
    cameraFrustum[5] = glm::normalize(rowW - rowZ);

    // Normalize planes
    for (auto i = 0U; i < 6U; ++i)
    {
        glm::vec3 planeNormal(cameraFrustum[i].x, cameraFrustum[i].y, cameraFrustum[i].z);
        const auto length = glm::length(planeNormal);
        cameraFrustum[i] = -cameraFrustum[i] / length;
    }

    return cameraFrustum;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
