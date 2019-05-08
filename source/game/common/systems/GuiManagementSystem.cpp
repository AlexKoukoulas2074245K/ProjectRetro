//
//  GuiManagementSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 07/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "GuiManagementSystem.h"
#include "../GameConstants.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/TextboxComponent.h"
#include "../../common/utils/StringUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/components/WindowSingletonComponent.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/MeshUtils.h"
#include "../../resources/ResourceLoadingService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

float GuiManagementSystem::GUI_TILE_DEFAULT_SIZE = 0.11f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

GuiManagementSystem::GuiManagementSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<TextboxComponent>();
    InitializeGuiState();
}

void GuiManagementSystem::VUpdateAssociatedComponents(const float) const
{
    auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& activeEntities = mWorld.GetActiveEntities();
    for (const auto& entityId: activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {
            inputStateComponent.mHasBeenConsumed = true;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void GuiManagementSystem::InitializeGuiState() const
{
    ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + "gui.png");

    auto windowSingletonComponent = mWorld.GetSingletonComponent<WindowSingletonComponent>();

    auto guiStateSingletonComponent                  = std::make_unique<GuiStateSingletonComponent>();
    guiStateSingletonComponent->mGlobalGuiTileWidth  = (GUI_TILE_DEFAULT_SIZE/GAME_TILE_SIZE)/windowSingletonComponent.mAspectRatio;
    guiStateSingletonComponent->mGlobalGuiTileHeight = GUI_TILE_DEFAULT_SIZE/GAME_TILE_SIZE;
    
    PopulateFontEntities(*guiStateSingletonComponent);
    
    mWorld.SetSingletonComponent<GuiStateSingletonComponent>(std::move(guiStateSingletonComponent));
}

void GuiManagementSystem::PopulateFontEntities(GuiStateSingletonComponent& guiStateComponent) const
{
    const auto fontCoordsResourceId = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_DATA_ROOT + "font_coords.dat");
    const auto& fontCoordsResource  = ResourceLoadingService::GetInstance().GetResource<DataFileResource>(fontCoordsResourceId);
    
    const auto fontCoordsSplitByNewline = StringSplit(fontCoordsResource.GetContents(), '\n');
    
    for (auto row = 0U; row < fontCoordsSplitByNewline.size(); ++row)
    {
        const auto fontCoordsLineSplitBySpace = StringSplit(fontCoordsSplitByNewline[row], ' ');
        for (auto col = 0U; col < fontCoordsLineSplitBySpace.size(); ++col)
        {
            const auto currentFontCharacter = fontCoordsLineSplitBySpace[col][0];
            guiStateComponent.mFontEntities[currentFontCharacter] = LoadMeshFromAtlasTexCoords
            (
                col,
                row,
                GUI_ATLAS_COLS,
                GUI_ATLAS_ROWS,
                false,
                GUI_COMPONENTS_MODEL_NAME
            );
        }
    }
    
    ResourceLoadingService::GetInstance().UnloadResource(fontCoordsResourceId);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
