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
#include "../../resources/MeshUtils.h"
#include "../../overworld/OverworldConstants.h"

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
    
    const std::string modelName = "camera_facing_quad";
    const auto atlasCols        = 8;
    const auto atlasRows        = 64;

    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations(spriteData.mAtlasColOffset, spriteData.mAtlasRowOffset, atlasCols, atlasRows, false, modelName, SOUTH_ANIMATION_NAME_ID, *renderableComponent);
    
    if (spriteData.mCharacterMovementType == CharacterMovementType::STATIC) return renderableComponent;
    
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations(spriteData.mAtlasColOffset + 1, spriteData.mAtlasRowOffset, atlasCols, atlasRows, false, modelName, NORTH_ANIMATION_NAME_ID, *renderableComponent);
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations(spriteData.mAtlasColOffset + 2, spriteData.mAtlasRowOffset, atlasCols, atlasRows, false, modelName, WEST_ANIMATION_NAME_ID, *renderableComponent);
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations(spriteData.mAtlasColOffset + 2, spriteData.mAtlasRowOffset, atlasCols, atlasRows, true, modelName, EAST_ANIMATION_NAME_ID, *renderableComponent);
    
    if (spriteData.mCharacterMovementType == CharacterMovementType::STATIONARY) return renderableComponent;
    
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations(spriteData.mAtlasColOffset + 3, spriteData.mAtlasRowOffset, atlasCols, atlasRows, true, modelName, SOUTH_ANIMATION_NAME_ID, *renderableComponent);
    renderableComponent->mAnimationsToMeshes[SOUTH_ANIMATION_NAME_ID].push_back(renderableComponent->mAnimationsToMeshes[SOUTH_ANIMATION_NAME_ID][0]);
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations(spriteData.mAtlasColOffset + 3, spriteData.mAtlasRowOffset, atlasCols, atlasRows, false, modelName, SOUTH_ANIMATION_NAME_ID, *renderableComponent);
    
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations(spriteData.mAtlasColOffset + 4, spriteData.mAtlasRowOffset, atlasCols, atlasRows, true, modelName, NORTH_ANIMATION_NAME_ID, *renderableComponent);
    renderableComponent->mAnimationsToMeshes[NORTH_ANIMATION_NAME_ID].push_back(renderableComponent->mAnimationsToMeshes[NORTH_ANIMATION_NAME_ID][0]);
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations(spriteData.mAtlasColOffset + 4, spriteData.mAtlasRowOffset, atlasCols, atlasRows, false, modelName, NORTH_ANIMATION_NAME_ID, *renderableComponent);
    
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations(spriteData.mAtlasColOffset + 5, spriteData.mAtlasRowOffset, atlasCols, atlasRows, false, modelName, WEST_ANIMATION_NAME_ID, *renderableComponent);
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations(spriteData.mAtlasColOffset + 5, spriteData.mAtlasRowOffset, atlasCols, atlasRows, true, modelName, EAST_ANIMATION_NAME_ID, *renderableComponent);
    
    return renderableComponent;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
