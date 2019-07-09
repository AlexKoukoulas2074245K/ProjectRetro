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
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/TextboxResidentComponent.h"
#include "../components/TransformComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/components/WindowSingletonComponent.h"
#include "../../resources/MeshUtils.h"
#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const glm::vec3 CHATBOX_POSITION                      = glm::vec3(0.0f, -0.6701f, 0.0f);
static const glm::vec3 ENCOUNTER_MAIN_MENU_TEXTBOX_POSITION  = glm::vec3(0.275f, -0.6701f, -0.2f);
static const glm::vec3 ENCOUNTER_FIGHT_MENU_TEXTBOX_POSITION = glm::vec3(0.1375f, -0.6701f, -0.2f);

static const int CHATBOX_COLS = 20;
static const int CHATBOX_ROWS = 6;

static const int ENCOUNTER_MAIN_MENU_TEXTBOX_COLS  = 12;
static const int ENCOUNTER_MAIN_MENU_TEXTBOX_ROWS  = 6;
static const int ENCOUNTER_FIGHT_MENU_TEXTBOX_COLS = 16;
static const int ENCOUNTER_FIGHT_MENU_TEXTBOX_ROWS = 6;

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
    const float textboxZ,
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

ecs::EntityId GetActiveTextboxEntityId
(
    const ecs::World& world
)
{
    const auto& guiStateComponent = world.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        return ecs::NULL_ENTITY_ID;
    }
    
    return guiStateComponent.mActiveTextboxesStack.top();
}

size_t GetFirstEmptyColumnInTextboxRow
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxRow,
    const ecs::World& world
)
{
    auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    
    assert((textboxRow > 0 && textboxRow < textboxComponent.mTextContent.size() - 1) && "Textbox row out of bounds");
    
    for (size_t textboxCol = 1U; textboxCol < textboxComponent.mTextContent[textboxRow].size() - 1; ++textboxCol)
    {
        const auto& characterEntry = textboxComponent.mTextContent[textboxRow][textboxCol];
        if (characterEntry.mCharacter == 0 && characterEntry.mEntityId == ecs::NULL_ENTITY_ID)
        {
            return textboxCol;
        }
    }
    
    return 0;
}

const std::vector<TextboxCharacterEntry>& GetTextboxRowContent
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxRow,
    ecs::World& world
)
{
    auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    
    assert((textboxRow > 0 && textboxRow < textboxComponent.mTextContent.size() - 1) && "Textbox row out of deletion bounds");
    
    return textboxComponent.mTextContent[textboxRow];
}

char GetCharacterAtTextboxCoords
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxCol,
    const size_t textboxRow,
    ecs::World& world
)
{
    auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    auto& textboxContent   = textboxComponent.mTextContent;
    
    assert((textboxRow > 0 && textboxRow < textboxContent.size() - 1) && "Textbox row out of writing bounds");
    assert((textboxCol > 0 && textboxCol < textboxContent[textboxRow].size() - 1) && "Textbox col out of writing bounds");
    
    return textboxContent[textboxRow][textboxCol].mCharacter;
}

ecs::EntityId CreateTextboxWithDimensions
(
    const TextboxType textboxType,
    const int textboxTileCols,
    const int textboxTileRows,
    const float textboxOriginX,
    const float textboxOriginY,
    const float textboxZ,
    ecs::World& world
)
{
    const auto textboxEntityId = world.CreateEntity();

    TextboxContent textContent(textboxTileRows);
    for (auto& row : textContent)
    {
        row.resize(textboxTileCols);
    }
        
    auto textboxComponent              = std::make_unique<TextboxComponent>();
    textboxComponent->mTextboxTileCols = textboxTileCols;
    textboxComponent->mTextboxTileRows = textboxTileRows;
    textboxComponent->mTextContent     = textContent;
    textboxComponent->mTextboxType     = textboxType;
    
    world.AddComponent<TextboxComponent>(textboxEntityId, std::move(textboxComponent));

    if (textboxType != TextboxType::BARE_TEXTBOX)
    {
        CreateTextboxComponents
        (
            textboxEntityId,
            textboxTileCols,
            textboxTileRows,
            textboxOriginX,
            textboxOriginY,
            textboxZ,
            world
        );
    }
    
    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mPosition = glm::vec3(textboxOriginX, textboxOriginY, textboxZ);
    world.AddComponent<TransformComponent>(textboxEntityId, std::move(transformComponent));
    
    if (textboxType != TextboxType::BARE_TEXTBOX)
    {
        auto& guiStateComponent = world.GetSingletonComponent<GuiStateSingletonComponent>();
        guiStateComponent.mActiveTextboxesStack.push(textboxEntityId);
    }    
    
    return textboxEntityId;
}

ecs::EntityId CreateChatbox
(
    ecs::World& world
)
{
    return CreateTextboxWithDimensions
    (
        TextboxType::CHATBOX,
        CHATBOX_COLS, 
        CHATBOX_ROWS, 
        CHATBOX_POSITION.x, 
        CHATBOX_POSITION.y,
        CHATBOX_POSITION.z, 
        world
    );
}

ecs::EntityId CreateEncounterMainMenuTextbox
(
    const MainMenuActionType actionTypeSelected,

    ecs::World& world
)
{
    const auto mainMenuTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        ENCOUNTER_MAIN_MENU_TEXTBOX_COLS,
        ENCOUNTER_MAIN_MENU_TEXTBOX_ROWS,
        ENCOUNTER_MAIN_MENU_TEXTBOX_POSITION.x,
        ENCOUNTER_MAIN_MENU_TEXTBOX_POSITION.y,
        ENCOUNTER_MAIN_MENU_TEXTBOX_POSITION.z,
        world
    );
    
    WriteTextAtTextboxCoords(mainMenuTextboxEntityId, "FIGHT", 2, 2, world);
    WriteTextAtTextboxCoords(mainMenuTextboxEntityId, "<>", 8, 2, world);
    WriteTextAtTextboxCoords(mainMenuTextboxEntityId, "ITEM", 2, 4, world);
    WriteTextAtTextboxCoords(mainMenuTextboxEntityId, "RUN", 8, 4, world);    

    auto cursorComponent = std::make_unique<CursorComponent>();
    const auto targetCursorCoords = sMainMenuActionTypesToCursorCoords.at(actionTypeSelected);

    cursorComponent->mCursorCol = targetCursorCoords.first;
    cursorComponent->mCursorRow = targetCursorCoords.second;

    cursorComponent->mCursorColCount = 2;
    cursorComponent->mCursorRowCount = 2;

    cursorComponent->mCursorDisplayHorizontalTileOffset     = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 2;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 6;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;

    WriteCharAtTextboxCoords
    (
        mainMenuTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;
    
    world.AddComponent<CursorComponent>(mainMenuTextboxEntityId, std::move(cursorComponent));
    
    return mainMenuTextboxEntityId;
}

ecs::EntityId CreateEncounterFightMenuTextbox
(
    const PokemonMoveSet& moveset,
    const int lastSelectedMoveIndex,
    ecs::World& world
)
{
    const auto fightMenuTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        ENCOUNTER_FIGHT_MENU_TEXTBOX_COLS,
        ENCOUNTER_FIGHT_MENU_TEXTBOX_ROWS,
        ENCOUNTER_FIGHT_MENU_TEXTBOX_POSITION.x,
        ENCOUNTER_FIGHT_MENU_TEXTBOX_POSITION.y,
        ENCOUNTER_FIGHT_MENU_TEXTBOX_POSITION.z,
        world
    );

    for (auto i = 0U; i < moveset.size(); ++i)
    {
        WriteTextAtTextboxCoords(fightMenuTextboxEntityId, moveset[i].mName.GetString(), 2, i + 1, world);
    }    

    auto cursorComponent = std::make_unique<CursorComponent>();    
    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = lastSelectedMoveIndex;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = 4;

    cursorComponent->mCursorDisplayHorizontalTileOffset     = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 1;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 1;

    WriteCharAtTextboxCoords
    (
        fightMenuTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = true;
    
    world.AddComponent<CursorComponent>(fightMenuTextboxEntityId, std::move(cursorComponent));

    return fightMenuTextboxEntityId;
}

void DestroyActiveTextbox
(
    ecs::World& world
)
{
    const auto textboxEntityId = GetActiveTextboxEntityId(world);
    assert(textboxEntityId != ecs::NULL_ENTITY_ID && "No active component available to destroy");
    
    auto& guiStateComponent = world.GetSingletonComponent<GuiStateSingletonComponent>();
    guiStateComponent.mActiveTextboxesStack.pop();
    
    const auto& entityIds = world.GetActiveEntities();
    for (const auto& entityId: entityIds)
    {
        if
        (
            world.HasComponent<TextboxResidentComponent>(entityId) &&
            world.GetComponent<TextboxResidentComponent>(entityId).mTextboxParentEntityId == textboxEntityId
        )
        {
            world.RemoveEntity(entityId);
        }
    }
    
    world.RemoveEntity(textboxEntityId);
}

void WriteCharAtTextboxCoords
(
     const ecs::EntityId textboxEntityId,
     const char character,
     const size_t textboxCol,
     const size_t textboxRow,
     ecs::World& world
)
{
    const auto& windowComponent           = world.GetSingletonComponent<WindowSingletonComponent>();
    const auto& guiStateComponent         = world.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& textboxTransformComponent = world.GetComponent<TransformComponent>(textboxEntityId);
    
    auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    auto& textboxContent   = textboxComponent.mTextContent;
    
    if (textboxContent[textboxRow][textboxCol].mEntityId != ecs::NULL_ENTITY_ID)
    {
        world.RemoveEntity(textboxContent[textboxRow][textboxCol].mEntityId);
    }
    
    const auto characterEntityId = world.CreateEntity();
    
    textboxComponent.mTextContent[textboxRow][textboxCol].mCharacter = character;
    textboxComponent.mTextContent[textboxRow][textboxCol].mEntityId  = characterEntityId;
    
    auto textboxResidentComponent                    = std::make_unique<TextboxResidentComponent>();
    textboxResidentComponent->mTextboxParentEntityId = textboxEntityId;
    
    world.AddComponent<TextboxResidentComponent>(characterEntityId, std::move(textboxResidentComponent));
    
    // Don't add transform or model components for whitespace character
    if (character == ' ')
    {
        return;
    }
    
    auto renderableComponent                    = std::make_unique<RenderableComponent>();
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + "gui.png");
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;
    renderableComponent->mAnimationsToMeshes[renderableComponent->mActiveAnimationNameId].push_back(guiStateComponent.mFontEntities.at(character));
    
    const auto tileWidth  = guiStateComponent.mGlobalGuiTileWidth;
    const auto tileHeight = guiStateComponent.mGlobalGuiTileHeight;

    // This is used for positional calculations only, otherwise the rendered dimensions
    // of the textbox itself will be wrong
    const auto tileHeightAccountingForAspect = tileHeight * windowComponent.mAspectRatio;

    const auto textboxTopLeftPoint = glm::vec3
    (
        textboxTransformComponent.mPosition.x - (textboxComponent.mTextboxTileCols * tileWidth) * 0.5f + tileWidth * 0.5f,
        textboxTransformComponent.mPosition.y + (textboxComponent.mTextboxTileRows * tileHeightAccountingForAspect) * 0.5f - tileHeightAccountingForAspect * 0.5f,
        0.0f
    );

    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mScale    = glm::vec3(guiStateComponent.mGlobalGuiTileWidth, guiStateComponent.mGlobalGuiTileHeight, 1.0f);
    transformComponent->mPosition = glm::vec3
    (
        textboxTopLeftPoint.x + tileWidth * textboxCol,
        textboxTopLeftPoint.y - tileHeightAccountingForAspect * textboxRow,
        textboxTransformComponent.mPosition.z -0.05f
     );
    
    world.AddComponent<RenderableComponent>(characterEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(characterEntityId, std::move(transformComponent));
}

void WriteTextAtTextboxCoords
(
     const ecs::EntityId textboxEntityId,
     const std::string& text,
     const size_t textboxCol,
     const size_t textboxRow,
     ecs::World& world
)
{
#ifndef NDEBUG
    auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    auto& textboxContent   = textboxComponent.mTextContent;

    assert((textboxCol + text.size() < textboxContent[textboxRow].size()) && "Word cannot fit in specified textbox coords");
#endif    
    
    auto wordIter = text.begin();
    for (auto x = textboxCol; x < textboxCol + text.size(); ++x)
    {
        WriteCharAtTextboxCoords
        (
             textboxEntityId,
             *wordIter++,
             x,
             textboxRow,
             world
        );
    }
}

void QueueDialogForTextbox
(
    const ecs::EntityId textboxEntityId,
    const std::string& rawDialogText,
    ecs::World& world
)
{
    auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    
    const auto dialogSplitByParagraph = StringSplit(rawDialogText, '@');
    for (const auto& paragraph: dialogSplitByParagraph)
    {
        TextboxQueuedLines queuedParagraph;
        const auto paragraphSplitToSentences = StringSplit(paragraph, '#');
        
        for (const auto& sentence: paragraphSplitToSentences)
        {
            if (sentence.size() == 0)
            {
                continue;
            }
            
            TextboxQueuedTextLine queuedLineCharacters;
            auto paddedSentence = sentence;
            while (paddedSentence.size() < textboxComponent.mTextContent[0].size() - 2)
            {
                paddedSentence += ' ';
            }
            
            for (const auto& character: paddedSentence)
            {
                queuedLineCharacters.push(character);
            }
            
            queuedParagraph.push(queuedLineCharacters);
        }
        
        textboxComponent.mQueuedDialog.push(queuedParagraph);
    }
    
    return;
}

void DeleteCharAtTextboxCoords
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxCol,
    const size_t textboxRow,
    ecs::World& world
)
{
    auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    
    const auto characterEntityId = textboxComponent.mTextContent[textboxRow][textboxCol].mEntityId;
    
    if (characterEntityId == ecs::NULL_ENTITY_ID)
    {
        return;
    }
    
    textboxComponent.mTextContent[textboxRow][textboxCol].mCharacter = 0;
    textboxComponent.mTextContent[textboxRow][textboxCol].mEntityId = ecs::NULL_ENTITY_ID;
    
    world.RemoveEntity(characterEntityId);
}

void DeleteTextAtTextboxRow
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxRow,
    ecs::World& world
)
{
    auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    
    for (size_t textboxCol = 1U; textboxCol < textboxComponent.mTextContent[textboxRow].size() - 1; ++textboxCol)
    {
        DeleteCharAtTextboxCoords(textboxEntityId, textboxCol, textboxRow, world);
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
    const float textboxZ,
    ecs::World& world
)
{
    const auto& guiStateSingletonComponent = world.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& windowSingletonComponent   = world.GetSingletonComponent<WindowSingletonComponent>();
    
    const auto guiTileWidth  = guiStateSingletonComponent.mGlobalGuiTileWidth;
    const auto guiTileHeight = guiStateSingletonComponent.mGlobalGuiTileHeight;
    
    // This is used for positional calculations only, otherwise the rendered dimensions
    // of the textbox itself will be wrong
    const auto guiTileHeightAccountingForAspect = guiTileHeight * windowSingletonComponent.mAspectRatio;

    // Calculate textbox corners in screen coords
    const auto textboxTopLeftPoint  = glm::vec3(textboxOriginX - (textboxTileCols * guiTileWidth) * 0.5f + guiTileWidth * 0.5f, textboxOriginY + (textboxTileRows * guiTileHeightAccountingForAspect) * 0.5f - guiTileHeightAccountingForAspect * 0.5f, textboxZ - 0.05f);
    const auto textboxTopRightPoint = glm::vec3(textboxOriginX + (textboxTileCols * guiTileWidth) * 0.5f - guiTileWidth * 0.5f, textboxOriginY + (textboxTileRows * guiTileHeightAccountingForAspect) * 0.5f - guiTileHeightAccountingForAspect * 0.5f, textboxZ - 0.05f);
    const auto textboxBotLeftPoint  = glm::vec3(textboxOriginX - (textboxTileCols * guiTileWidth) * 0.5f + guiTileWidth * 0.5f, textboxOriginY - (textboxTileRows * guiTileHeightAccountingForAspect) * 0.5f + guiTileHeightAccountingForAspect * 0.5f, textboxZ - 0.05f);
    const auto textboxBotRightPoint = glm::vec3(textboxOriginX + (textboxTileCols * guiTileWidth) * 0.5f - guiTileWidth * 0.5f, textboxOriginY - (textboxTileRows * guiTileHeightAccountingForAspect) * 0.5f + guiTileHeightAccountingForAspect * 0.5f, textboxZ - 0.05f);
    
    // Calculate filler components' dimensions
    const auto textboxHorizontalFillerWidth = guiTileWidth  * (textboxTileCols - 2);
    const auto textboxVerticalFillerHeight  = guiTileHeight * (textboxTileRows - 2);
    
    // Calculate filler components' screen coords
    const auto textboxTopFillerCoords        = glm::vec3(textboxOriginX, textboxTopLeftPoint.y, textboxZ - 0.05f);
    const auto textboxBotFillerCoords        = glm::vec3(textboxOriginX, textboxBotLeftPoint.y, textboxZ - 0.05f);
    const auto textboxLeftFillerCoords       = glm::vec3(textboxTopLeftPoint.x, textboxOriginY, textboxZ - 0.05f);
    const auto textboxRightFillerCoords      = glm::vec3(textboxTopRightPoint.x, textboxOriginY, textboxZ - 0.05f);
    const auto textboxBackgroundFillerCoords = glm::vec3(textboxOriginX, textboxOriginY, textboxZ - 0.02f);
    
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
    renderableComponent->mAffectedByPerspective = false;

    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations
    (
        atlasCol, 
        atlasRow, 
        GUI_ATLAS_COLS,
        GUI_ATLAS_ROWS,
        false, 
        GUI_COMPONENTS_MODEL_NAME, 
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
