//
//  TextboxUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 05/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TextboxUtils.h"
#include "../../ECS.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/TextboxComponent.h"
#include "../components/TextboxResidentComponent.h"
#include "../components/TransformComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/MeshUtils.h"
#include "../utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string TEXTBOX_COMPONENTS_MODEL_NAME = "camera_facing_quad";
static const int GUI_ATLAS_COLS = 16;
static const int GUI_ATLAS_ROWS = 16;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static void CreateTextboxComponents
(
    const ecs::EntityId textboxEntityId,
    const int textboxTileCols,
    const int textboxTileRows,
    const float textboxOriginX,
    const float textboxOriginY,
    ecs::World& world
);

static ecs::EntityId CreateTextboxComponentFromAtlasCoords
(
    const ecs::EntityId textboxEntityId,
    const int atlasCol,
    const int atlasRow,
    const glm::vec3& componentScale,
    const glm::vec3& componentPosition,
    ecs::World& world
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId CreateTextboxWithDimensions
(
    const int textboxTileCols,
    const int textboxTileRows,
    const float textboxOriginX,
    const float textboxOriginY,
    ecs::World& world
)
{
    assert(textboxTileCols >= TEXTBOX_MIN_TILE_COLS && "Invalid textbox tile cols were supplied");
    assert(textboxTileRows >= TEXTBOX_MIN_TILE_ROWS && "Invalid textbox tile rows were supplied");
    
    const auto textboxEntityId = world.CreateEntity();

    std::vector<std::vector<char>> textContent(textboxTileRows);    
    for (auto& row : textContent)
    {
        row.resize(textboxTileCols);
    }
        
    auto textboxComponent              = std::make_unique<TextboxComponent>();
    textboxComponent->mTextboxTileCols = textboxTileCols;
    textboxComponent->mTextboxTileRows = textboxTileRows;
    textboxComponent->mTextContent     = textContent;        
    world.AddComponent<TextboxComponent>(textboxEntityId, std::move(textboxComponent));

    CreateTextboxComponents
    (
        textboxEntityId,
        textboxTileCols,
        textboxTileRows,
        textboxOriginX,
        textboxOriginY,
        world
    );

    auto& guiStateComponent               = world.GetSingletonComponent<GuiStateSingletonComponent>();
    guiStateComponent.mActiveGuiComponent = textboxEntityId;
    
    return textboxEntityId;
}

void WriteTextAtTextboxCoords
(
    const std::string& text,
    const size_t textboxCol,
    const size_t textboxRow,
    std::vector<std::vector<char>>& textboxContent
)
{
    assert((textboxRow > 0 && textboxRow < textboxContent.size() - 1) && "Textbox row out of writing bounds bounds");
    assert((textboxCol > 0 && textboxCol < textboxContent[textboxRow].size() - 1) && "Textbox col out of writing bounds");
    assert((textboxCol + text.size() < textboxContent[textboxRow].size()) && "Word cannot fit in specified textbox coords");
    
    auto wordIter = text.begin();
    for (auto x = textboxCol; x < textboxCol + text.size(); ++x)
    {
        textboxContent[textboxRow][x] = *wordIter++;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void CreateTextboxComponents
(
    const ecs::EntityId textboxEntityId,
    const int textboxTileCols,
    const int textboxTileRows,
    const float textboxOriginX,
    const float textboxOriginY,
    ecs::World& world
)
{
    const auto& guiStateSingletonComponent = world.GetSingletonComponent<GuiStateSingletonComponent>();

    const auto guiTileWidth  = guiStateSingletonComponent.mGlobalGuiTileWidth;
    const auto guiTileHeight = guiStateSingletonComponent.mGlobalGuiTileHeight;

    // Calculate textbox corners in screen coords
    const auto textboxTopLeftPoint  = glm::vec3(textboxOriginX - (textboxTileCols * guiTileWidth) * 0.5f, textboxOriginY + (textboxTileRows * guiTileHeight) * 0.5f, 0.0f);
    const auto textboxTopRightPoint = glm::vec3(textboxOriginX + (textboxTileCols * guiTileWidth) * 0.5f, textboxOriginY + (textboxTileRows * guiTileHeight) * 0.5f, 0.0f);
    const auto textboxBotLeftPoint  = glm::vec3(textboxOriginX - (textboxTileCols * guiTileWidth) * 0.5f, textboxOriginY - (textboxTileRows * guiTileHeight) * 0.5f, 0.0f);
    const auto textboxBotRightPoint = glm::vec3(textboxOriginX + (textboxTileCols * guiTileWidth) * 0.5f, textboxOriginY - (textboxTileRows * guiTileHeight) * 0.5f, 0.0f);
    
    // Calculate filler components' dimensions
    const auto textboxHorizontalFillerWidth = guiTileWidth  * (textboxTileCols/2 + 1.5f);
    const auto textboxVerticalFillerHeight  = guiTileHeight * (textboxTileRows/2 + 1.5f);
    
    // Calculate filler components' screen coords
    const auto textboxTopFillerCoords        = glm::vec3(textboxOriginX, textboxOriginY + (textboxTileRows * guiTileHeight) * 0.5f, 0.0f);
    const auto textboxBotFillerCoords        = glm::vec3(textboxOriginX, textboxOriginY - (textboxTileRows * guiTileHeight) * 0.5f, 0.0f);
    const auto textboxLeftFillerCoords       = glm::vec3(textboxOriginX - (textboxTileCols * guiTileWidth) * 0.5f, textboxOriginY, 0.0f);
    const auto textboxRightFillerCoords      = glm::vec3(textboxOriginX + (textboxTileCols * guiTileWidth) * 0.5f, textboxOriginY, 0.0f);
    const auto textboxBackgroundFillerCoords = glm::vec3(textboxOriginX, textboxOriginY, 0.0f);
    
    CreateTextboxComponentFromAtlasCoords(textboxEntityId, 2, 6, glm::vec3(guiTileWidth, guiTileHeight, 1.0f), textboxTopLeftPoint, world);
    CreateTextboxComponentFromAtlasCoords(textboxEntityId, 4, 6, glm::vec3(guiTileWidth, guiTileHeight, 1.0f), textboxTopRightPoint, world);
    CreateTextboxComponentFromAtlasCoords(textboxEntityId, 8, 6, glm::vec3(guiTileWidth, guiTileHeight, 1.0f), textboxBotLeftPoint, world);
    CreateTextboxComponentFromAtlasCoords(textboxEntityId, 10, 6, glm::vec3(guiTileWidth, guiTileHeight, 1.0f), textboxBotRightPoint, world);
    CreateTextboxComponentFromAtlasCoords(textboxEntityId, 3, 6, glm::vec3(textboxHorizontalFillerWidth, guiTileHeight, 1.0f), textboxTopFillerCoords, world);
    CreateTextboxComponentFromAtlasCoords(textboxEntityId, 3, 6, glm::vec3(textboxHorizontalFillerWidth, guiTileHeight, 1.0f), textboxBotFillerCoords, world);
    CreateTextboxComponentFromAtlasCoords(textboxEntityId, 5, 6, glm::vec3(guiTileWidth, textboxVerticalFillerHeight, 1.0f), textboxLeftFillerCoords, world);
    CreateTextboxComponentFromAtlasCoords(textboxEntityId, 7, 6, glm::vec3(guiTileWidth, textboxVerticalFillerHeight, 1.0f), textboxRightFillerCoords, world);
    CreateTextboxComponentFromAtlasCoords(textboxEntityId, 6, 6, glm::vec3(textboxHorizontalFillerWidth, textboxVerticalFillerHeight, 1.0f), textboxBackgroundFillerCoords, world);
}

ecs::EntityId CreateTextboxComponentFromAtlasCoords
(
    const ecs::EntityId textboxEntityId,
    const int atlasCol,
    const int atlasRow,
    const glm::vec3& componentScale,
    const glm::vec3& componentPosition,
    ecs::World& world
)
{
    const auto componentEntityId = world.CreateEntity();
    
    auto renderableComponent                    = std::make_unique<RenderableComponent>();
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + "gui.png");
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");    

    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations
    (
        atlasCol, 
        atlasRow, 
        GUI_ATLAS_COLS,
        GUI_ATLAS_ROWS,
        false, 
        TEXTBOX_COMPONENTS_MODEL_NAME, 
        renderableComponent->mActiveAnimationNameId,
        *renderableComponent
    );

    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = componentPosition;
    transformComponent->mScale    = componentScale;

    auto textboxResidentComponent                    = std::make_unique<TextboxResidentComponent>();
    textboxResidentComponent->mTextboxParentEntityId = textboxEntityId;

    world.AddComponent<RenderableComponent>(componentEntityId, std::move(renderableComponent));
    world.AddComponent<TextboxResidentComponent>(componentEntityId, std::move(textboxResidentComponent));
    world.AddComponent<TransformComponent>(componentEntityId, std::move(transformComponent));

    return componentEntityId;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
