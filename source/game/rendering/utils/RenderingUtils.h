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

#include "CameraUtils.h"
#include "../../common/GameConstants.h"
#include "../../common/utils/MathUtils.h"

#include <SDL_stdinc.h>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

using EntityId = long long;
namespace ecs
{
    class World;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

bool IsMeshInsideCameraFrustum
(
    const glm::vec3& meshPosition,
    const glm::vec3& meshScale,
    const glm::vec3& meshDimensions,
    const CameraFrustum& cameraFrustum
);

void OverrideEntityPrimaryColorsBasedOnAnotherEntityPrimaryColors
(
    const ecs::EntityId entityToOverridePrimaryColors,
    const ecs::EntityId entityToExtractPrimarColorsFrom,
    const ecs::World& world
);

void GetPrimaryLightAndPrimaryDarkColorsFromSet
(
    const std::vector<Uint32>& colorSet,
    glm::vec4& outPrimaryLightColor,
    glm::vec4& outPrimaryDarkColor
);

glm::vec4 Uint32ColorToVec4(const Uint32);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* RenderingUtils_h */
