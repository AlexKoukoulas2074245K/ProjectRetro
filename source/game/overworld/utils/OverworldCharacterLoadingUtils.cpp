//
//  OverworldCharacterLoadingUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OverworldCharacterLoadingUtils.h"
#include "../../common/GameConstants.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/StringUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../resources/TextureUtils.h"
#include "../../overworld/OverworldConstants.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static std::string CreateTexCoordInjectedModelPath
(
    const std::string& modelName,
    const std::vector<glm::vec2>& texCoords
);

static void LoadMeshFromTexCoordsAndAddToRenderableComponent
(
    const int meshAtlasCol,
    const int meshAtlasRow,
    const bool horFlipped,
    const StringId& animationNameId,
    RenderableComponent& renderableComponent
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<RenderableComponent> CreateRenderableComponentForSprite(const CharacterSpriteData& spriteData)
{
    auto renderableComponent = std::make_unique<RenderableComponent>();
    
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + "characters.png");
    renderableComponent->mActiveAnimationNameId = NORTH_ANIMATION_NAME_ID;
    renderableComponent->mShaderNameId          = StringId("basic");
    renderableComponent->mAffectedByPerspective = false;
    
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset, spriteData.mAtlasRowOffset, false, SOUTH_ANIMATION_NAME_ID, *renderableComponent);
    
    if (spriteData.mCharacterMovementType == CharacterMovementType::STATIC) return renderableComponent;
    
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 1, spriteData.mAtlasRowOffset, false, NORTH_ANIMATION_NAME_ID, *renderableComponent);
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 2, spriteData.mAtlasRowOffset, false, WEST_ANIMATION_NAME_ID, *renderableComponent);
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 2, spriteData.mAtlasRowOffset, true, EAST_ANIMATION_NAME_ID, *renderableComponent);
    
    if (spriteData.mCharacterMovementType == CharacterMovementType::STATIONARY) return renderableComponent;
    
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 3, spriteData.mAtlasRowOffset, true, SOUTH_ANIMATION_NAME_ID, *renderableComponent);
    renderableComponent->mAnimationsToMeshes[SOUTH_ANIMATION_NAME_ID].push_back(renderableComponent->mAnimationsToMeshes[SOUTH_ANIMATION_NAME_ID][0]);
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 3, spriteData.mAtlasRowOffset, false, SOUTH_ANIMATION_NAME_ID, *renderableComponent);
    
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 4, spriteData.mAtlasRowOffset, true, NORTH_ANIMATION_NAME_ID, *renderableComponent);
    renderableComponent->mAnimationsToMeshes[NORTH_ANIMATION_NAME_ID].push_back(renderableComponent->mAnimationsToMeshes[NORTH_ANIMATION_NAME_ID][0]);
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 4, spriteData.mAtlasRowOffset, false, NORTH_ANIMATION_NAME_ID, *renderableComponent);
    
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 5, spriteData.mAtlasRowOffset, false, WEST_ANIMATION_NAME_ID, *renderableComponent);
    LoadMeshFromTexCoordsAndAddToRenderableComponent(spriteData.mAtlasColOffset + 5, spriteData.mAtlasRowOffset, true, EAST_ANIMATION_NAME_ID, *renderableComponent);
    
    return renderableComponent;
}

std::string CreateTexCoordInjectedModelPath(const std::string& modelName, const std::vector<glm::vec2>& texCoords)
{
    std::string path = ResourceLoadingService::RES_MODELS_ROOT + modelName + "[";
    
    path += std::to_string(texCoords[0].x) + "," + std::to_string(texCoords[0].y);
    
    for (auto i = 1U; i < texCoords.size(); ++i)
    {
        path += "-" + std::to_string(texCoords[i].x) + "," + std::to_string(texCoords[i].y);
    }
    
    path += "].obj";
    
    return path;
}

void LoadMeshFromTexCoordsAndAddToRenderableComponent(const int meshAtlasCol, const int meshAtlasRow, const bool horFlipped, const StringId& animationNameId, RenderableComponent& renderableComponent)
{
    const auto atlasCols = 8;
    const auto atlasRows = 64;
    
    auto correctedMeshCol = meshAtlasCol;
    auto correctedMeshRow = meshAtlasRow;
    
    if (correctedMeshCol >= atlasCols)
    {
        correctedMeshCol %= atlasCols;
        correctedMeshRow++;
    }
    
    const auto texCoords = CalculateTextureCoordsFromColumnAndRow(correctedMeshCol, correctedMeshRow, atlasCols, atlasRows, horFlipped);
    const auto meshPath  = CreateTexCoordInjectedModelPath("camera_facing_quad", texCoords);
    
    renderableComponent.mAnimationsToMeshes[animationNameId].push_back(ResourceLoadingService::GetInstance().LoadResource(meshPath));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
