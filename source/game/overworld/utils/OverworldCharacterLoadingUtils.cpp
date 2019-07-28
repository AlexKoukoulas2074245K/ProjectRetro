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
#include "LevelUtils.h"
#include "../../common/GameConstants.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/StringUtils.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../resources/MeshUtils.h"
#include "../../overworld/OverworldConstants.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId CreatePlayerOverworldSprite
(
    const ecs::EntityId levelEntityId,
    const Direction direction,
    const int currentCol,
    const int currentRow,
    ecs::World& world
)
{
    const auto playerEntityId = world.CreateEntity();
    
    auto animationComponent = std::make_unique<AnimationTimerComponent>();
    animationComponent->mAnimationTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);
    animationComponent->mAnimationTimer->Pause();
    
    auto movementStateComponent = std::make_unique<MovementStateComponent>();
    movementStateComponent->mCurrentCoords = TileCoords(currentCol, currentRow);
    
    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = TileCoordsToPosition(currentCol, currentRow);
    
    auto directionComponent = std::make_unique<DirectionComponent>();
    directionComponent->mDirection = direction;
    
    world.AddComponent<AnimationTimerComponent>(playerEntityId, std::move(animationComponent));
    world.AddComponent<DirectionComponent>(playerEntityId, std::move(directionComponent));
    world.AddComponent<MovementStateComponent>(playerEntityId, std::move(movementStateComponent));
    world.AddComponent<PlayerTagComponent>(playerEntityId, std::make_unique<PlayerTagComponent>());
    world.AddComponent<RenderableComponent>(playerEntityId, CreateRenderableComponentForSprite(CharacterSpriteData(CharacterMovementType::DYNAMIC, 6, 14)));
    world.AddComponent<TransformComponent>(playerEntityId, std::move(transformComponent));
    
    auto& levelModelComponent = world.GetComponent<LevelModelComponent>(levelEntityId);
    GetTile(currentCol, currentRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = playerEntityId;
    GetTile(currentCol, currentRow, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::PLAYER;
    
    auto& playerStateComponent = world.GetSingletonComponent<PlayerStateSingletonComponent>();
    playerStateComponent.mLastOverworldLevelName = levelModelComponent.mLevelName;
    
    return playerEntityId;
}

std::unique_ptr<RenderableComponent> CreateRenderableComponentForSprite
(
    const CharacterSpriteData& spriteData
)
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
