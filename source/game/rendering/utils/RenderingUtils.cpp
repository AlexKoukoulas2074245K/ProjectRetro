//
//  RenderingUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "RenderingUtils.h"
#include "../components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../resources/TextureResource.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

bool IsMeshInsideCameraFrustum
(
    const glm::vec3& meshPosition,
    const glm::vec3& meshScale,
    const glm::vec3& meshDimensions, 
    const CameraFrustum& cameraFrustum
)
{    
    const auto scaledMeshDimensions = meshDimensions * meshScale;
    const auto frustumCheckSphereRadius = math::Max(scaledMeshDimensions.x, math::Max(scaledMeshDimensions.y, scaledMeshDimensions.z));

    for (auto i = 0U; i < 6U; ++i)
    {
        float dist = 
            cameraFrustum[i].x * meshPosition.x +
            cameraFrustum[i].y * meshPosition.y +
            cameraFrustum[i].z * meshPosition.z +
            cameraFrustum[i].w - frustumCheckSphereRadius;

        if (dist > 0) return false;
    }
  
    return true;
}

void OverridePrimaryColorsBasedOnAnotherEntityPrimaryColors
(
    RenderableComponent& renderableComponent,
    const ecs::EntityId entityToExtractPrimarColorsFrom,
    const ecs::World& world
)
{
    const auto& otherRenderingComponent = world.GetComponent<RenderableComponent>(entityToExtractPrimarColorsFrom);
    const auto& otherTextureResource    = ResourceLoadingService::GetInstance().GetResource<TextureResource>(otherRenderingComponent.mTextureResourceId);
    
    const auto& colorSet  = otherTextureResource.GetColorSet();
    
    glm::vec4 primaryLightColor, primaryDarkColor;
    
    GetPrimaryLightAndPrimaryDarkColorsFromSet(colorSet, primaryLightColor, primaryDarkColor);
    
    renderableComponent.mShouldOverrideDarkAndLightColor = true;
    renderableComponent.mOverriddenLightColor            = primaryLightColor;
    renderableComponent.mOverriddenDarkColor             = primaryDarkColor;
}

void OverrideEntityPrimaryColorsBasedOnAnotherEntityPrimaryColors
(
    const ecs::EntityId entityToOverridePrimaryColors,
    const ecs::EntityId entityToExtractPrimarColorsFrom,
    const ecs::World& world
)
{
    auto& overridenRenderableComponent = world.GetComponent<RenderableComponent>(entityToOverridePrimaryColors);
    OverridePrimaryColorsBasedOnAnotherEntityPrimaryColors(overridenRenderableComponent, entityToExtractPrimarColorsFrom, world);
}

void GetPrimaryLightAndPrimaryDarkColorsFromSet
(
    const std::vector<Uint32>& colorSet,
    glm::vec4& outPrimaryLightColor,
    glm::vec4& outPrimaryDarkColor
)
{
    const auto colorCount = static_cast<int>(colorSet.size());

    if (colorCount == 5)
    {
#ifdef _WIN32
        const auto uint32Color = colorSet[colorCount - 1];
        if 
        (
            ((uint32Color >> 0) & 0xFF) == 0xF8 && 
            ((uint32Color >> 8) & 0xFF) == 0xF8 &&
            ((uint32Color >> 16) & 0xFF) == 0xF8
        )
        {
            outPrimaryLightColor = Uint32ColorToVec4(colorSet[math::Max(0, colorCount - 2)]);
            outPrimaryDarkColor = Uint32ColorToVec4(colorSet[math::Max(0, colorCount - 3)]);
        }
        else
        {
            outPrimaryLightColor = Uint32ColorToVec4(colorSet[math::Max(0, colorCount - 1)]);
            outPrimaryDarkColor = Uint32ColorToVec4(colorSet[math::Max(0, colorCount - 2)]);
        }
        
#else
        outPrimaryLightColor = Uint32ColorToVec4(colorSet[math::Max(0, colorCount - 2)]);
        outPrimaryDarkColor = Uint32ColorToVec4(colorSet[math::Max(0, colorCount - 3)]);
#endif
    }
    else
    {
        outPrimaryLightColor = Uint32ColorToVec4(colorSet[math::Max(0, colorCount - 1)]);
        outPrimaryDarkColor = Uint32ColorToVec4(colorSet[math::Max(0, colorCount - 2)]);
    }
}

glm::vec4 Uint32ColorToVec4(const Uint32 intColor)
{
#ifdef _WIN32
    return glm::vec4
    (
        ((intColor >> 0) & 0xFF) / 255.0f,
        ((intColor >> 8) & 0xFF) / 255.0f,
        ((intColor >> 16) & 0xFF) / 255.0f,
        ((intColor >> 24) & 0xFF) / 255.0f
    );
#else
    return glm::vec4
    (
        ((intColor >> 16) & 0xFF) / 255.0f,
        ((intColor >> 8)  & 0xFF) / 255.0f,
        ((intColor >> 0)  & 0xFF) / 255.0f,
        ((intColor >> 24) & 0xFF) / 255.0f
    );
#endif
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
