//
//  OverworldCharacterLoadingUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef OverworldCharacterLoadingUtils_h
#define OverworldCharacterLoadingUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/GameConstants.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class RenderableComponent;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class CharacterSpriteData
{
public:
    CharacterSpriteData(const CharacterMovementType characterMovementType, const int atlasColOffset, const int atlasRowOffset)
    : mCharacterMovementType(characterMovementType)
    , mAtlasColOffset(atlasColOffset)
    , mAtlasRowOffset(atlasRowOffset)
    {
    }
    
    const CharacterMovementType mCharacterMovementType;
    const int mAtlasColOffset;
    const int mAtlasRowOffset;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<RenderableComponent> CreateRenderableComponentForSprite(const CharacterSpriteData& spriteData);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* OverworldCharacterLoadingUtils_h */
