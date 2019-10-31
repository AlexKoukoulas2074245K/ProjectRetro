//
//  TextboxUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 05/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MilestoneUtils.h"
#include "PokedexUtils.h"
#include "TextboxUtils.h"
#include "../../ECS.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/ItemMenuStateComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TextboxResidentComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/MathUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/StringUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/components/WindowSingletonComponent.h"
#include "../../resources/MeshUtils.h"


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 CHATBOX_POSITION                      = glm::vec3(0.0f, -0.6701f, 0.0f);
const glm::vec3 ENCOUNTER_FIGHT_MENU_TEXTBOX_POSITION = glm::vec3(0.1375f, -0.6701f, -0.2f);

static const glm::vec3 OVERWORLD_MAIN_MENU_TEXTBOX_POSITION_WITH_POKEDEX    = glm::vec3(0.35f, 0.122f, 0.1f);
static const glm::vec3 OVERWORLD_MAIN_MENU_TEXTBOX_POSITION_WITHOUT_POKEDEX = glm::vec3(0.35f, 0.23f, 0.1f);
static const glm::vec3 ENCOUNTER_MAIN_MENU_TEXTBOX_POSITION                 = glm::vec3(0.275f, -0.6701f, -0.2f);
static const glm::vec3 ENCOUNTER_FIGHT_MENU_MOVE_INFO_TEXTBOX_POSITION      = glm::vec3(-0.31f, -0.1801f, -0.4f);
static const glm::vec3 POKEMON_STATS_DISPLAY_TEXTBOX_POSITION               = glm::vec3(0.0f, 0.0f, -0.8f);
static const glm::vec3 POKEDEX_POKEMON_ENTRY_DISPLAY_TEXTBOX_POSITION       = glm::vec3(0.005f, 0.0f, -0.8f);
static const glm::vec3 ITEM_MENU_TEXTBOX_POSITION                           = glm::vec3(0.1337f, 0.167f, -0.1f);
static const glm::vec3 MART_MONEY_TEXTBOX_POSITION                          = glm::vec3(0.377200305f, 0.836796045f, 0.0f);
static const glm::vec3 MART_MENU_TEXTBOX_POSITION                           = glm::vec3(-0.309200227f, 0.616698325f, 0.0f);
static const glm::vec3 MART_ITEM_QUANTITY_TEXTBOX_POSITION                  = glm::vec3(0.232700080, -0.160900041, -0.4f);
static const glm::vec3 PC_MAIN_OPTIONS_TEXTBOX_POSITION                     = glm::vec3(-0.134f, 0.56f, 0.0f);
static const glm::vec3 PC_POKEMON_SYSTEM_OPTIONS_TEXTBOX_POSITION           = glm::vec3(-0.206199840f, 0.452099757f, -0.1f);
static const glm::vec3 PC_POKEMON_SYSTEM_POKEMON_LIST_TEXTBOX_POSITION      = glm::vec3(0.1337f, 0.167f, -0.2f);
static const glm::vec3 PC_POKEMON_SELECTED_OPTIONS_TEXTBOX_POSITION         = glm::vec3(0.304f, -0.561f, -0.4f);
static const glm::vec3 POKEDEX_MAIN_VIEW_INVISIBLE_LIST_TEXTBOX_POSITION    = glm::vec3(-0.171200007f, 0.0f, -0.1f);
static const glm::vec3 BLACKBOARD_TEXTBOX_POSITION                          = glm::vec3(-0.272698253f, 0.564099908f, -0.4f);
static const glm::vec3 SAVE_SCREEN_PLAYER_STATS_TEXTBOX_POSITION            = glm::vec3(0.1337f, 0.451719970f, -0.1f);
static const glm::vec3 NAME_SELECTION_CHARACTERS_ENCLOSING_TEXTBOX_POSITION = glm::vec3(0.0f, 0.0f, 0.0f);
static const glm::vec3 NAME_SELECTION_CHARACTERS_INVISIBLE_TEXTBOX_POSITION = glm::vec3(0.0f, -0.1f, -0.1f);
static const glm::vec3 NAME_SELECTION_TITLE_INVISIBLE_TEXTBOX_POSITION      = glm::vec3(0.0f, 0.7646f, -0.1f);
static const int CHATBOX_COLS = 20;
static const int CHATBOX_ROWS = 6;

static const int ITEM_MENU_TEXTBOX_COLS = 16;
static const int ITEM_MENU_TEXTBOX_ROWS = 11;

static const int MART_MENU_TEXTBOX_COLS = 11;
static const int MART_MENU_TEXTBOX_ROWS = 7;

static const int MART_MONEY_TEXTBOX_COLS = 9;
static const int MART_MONEY_TEXTBOX_ROWS = 3;

static const int MART_ITEM_QUANTITY_TEXTBOX_COLS = 13;
static const int MART_ITEM_QUANTITY_TEXTBOX_ROWS = 3;

static const int PC_MAIN_OPTIONS_TEXTBOX_COLS = 16;
static const int PC_MAIN_OPTIONS_TEXTBOX_ROWS = 8;

static const int PC_POKEMON_SYSTEM_OPTIONS_TEXTBOX_COLS = 14;
static const int PC_POKEMON_SYSTEM_OPTIONS_TEXTBOX_ROWS = 10;

static const int PC_POKEMON_SYSTEM_POKEMON_LIST_TEXTBOX_COLS = 16;
static const int PC_POKEMON_SYSTEM_POKEMON_LIST_TEXTBOX_ROWS = 11;

static const int PC_POKEMON_SELECTED_OPTIONS_TEXTBOX_COLS = 11;
static const int PC_POKEMON_SELECTED_OPTIONS_TEXTBOX_ROWS = 8;

static const int YES_NO_TEXTBOX_COLS = 6;
static const int YES_NO_TEXTBOX_ROWS = 5;

static const int USE_TOSS_TEXTBOX_COLS = 7;
static const int USE_TOSS_TEXTBOX_ROWS = 5;

static const int HEAL_CANCEL_TEXTBOX_COLS = 9;
static const int HEAL_CANCEL_TEXTBOX_ROWS = 6;

static const int OVERWORLD_MAIN_MENU_TEXTBOX_COLS_WITH_POKEDEX = 10;
static const int OVERWORLD_MAIN_MENU_TEXTBOX_ROWS_WITH_POKEDEX = 16;

static const int ENCOUNTER_MAIN_MENU_TEXTBOX_COLS  = 12;
static const int ENCOUNTER_MAIN_MENU_TEXTBOX_ROWS  = 6;

static const int ENCOUNTER_FIGHT_MENU_TEXTBOX_COLS = 16;
static const int ENCOUNTER_FIGHT_MENU_TEXTBOX_ROWS = 6;

static const int ENCOUNTER_FIGHT_MENU_MOVE_INFO_TEXTBOX_COLS = 11;
static const int ENCOUNTER_FIGHT_MENU_MOVE_INFO_TEXTBOX_ROWS = 5;

static const int POKEMON_MINI_STATS_DISPLAY_TEXTBOX_COLS = 11;
static const int POKEMON_MINI_STATS_DISPLAY_TEXTBOX_ROWS = 10;

static const int POKEMON_STATS_DISPLAY_TEXTBOX_COLS = 20;
static const int POKEMON_STATS_DISPLAY_TEXTBOX_ROWS = 18;

static const int POKEDEX_POKEMON_ENTRY_DISPLAY_TEXTBOX_COLS = 20;
static const int POKEDEX_POKEMON_ENTRY_DISPLAY_TEXTBOX_ROWS = 18;

static const int BLACKBOARD_TEXTBOX_COLS = 12;
static const int BLACKBOARD_TEXTBOX_ROWS = 8;

static const int SAVE_SCREEN_PLAYER_STATS_TEXTBOX_COLS = 16;
static const int SAVE_SCREEN_PLAYER_STATS_TEXTBOX_ROWS = 10;

static const int NAME_SELECTION_CHARACTERS_ENCLOSING_TEXTBOX_COLS = 20;
static const int NAME_SELECTION_CHARACTERS_ENCLOSING_TEXTBOX_ROWS = 11;

static const int NAME_SELECTION_CHARACTERS_INVISIBLE_TEXTBOX_COLS = 18;
static const int NAME_SELECTION_CHARACTERS_INVISIBLE_TEXTBOX_ROWS = 11;

static const int NAME_SELECTION_TITLE_INVISIBLE_TEXTBOX_COLS = 20;
static const int NAME_SELECTION_TITLE_INVISIBLE_TEXTBOX_ROWS = 3;

static const int POKEDEX_MAIN_VIEW_INVISIBLE_LIST_TEXTBOX_COLS = 15;
static const int POKEDEX_MAIN_VIEW_INVISIBLE_LIST_TEXTBOX_ROWS = 14;

static const std::unordered_map<SpecialCharacter, TileCoords> SPECIAL_CHARACTERS_TO_ATLAS_COORDS =
{
    { SpecialCharacter::CHARACTER_STAND_UP, TileCoords(2, 7) },
    { SpecialCharacter::CHARACTER_STAND_NORMAL, TileCoords(3, 7) },
    { SpecialCharacter::POKEDEX_CAUGHT_BALL, TileCoords(4, 7) }
};

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

static void ToggleCursoredTextboxActivityDisplay
(
    const ecs::EntityId textboxEntityId,
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

std::string GetTextboxRowString
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxRow,
    ecs::World& world
)
{
#ifndef NDEBUG
    auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
#endif
    assert((textboxRow > 0 && textboxRow < textboxComponent.mTextContent.size() - 1) && "Textbox row out of deletion bounds");
    
    std::string textboxRowContent = "";
    for (const auto& characterEntry: GetTextboxRowContent(textboxEntityId, textboxRow, world))
    {
        textboxRowContent.push_back(characterEntry.mCharacter);
    }
    
    return textboxRowContent;
}

std::vector<ecs::EntityId> GetAllTextboxResidentComponents
(
    const ecs::EntityId textboxEntityId,
    ecs::World& world
)
{
    std::vector<ecs::EntityId> residentComponents;
    for (const auto& entityId: world.GetActiveEntities())
    {
        if
        (
            world.HasComponent<TextboxResidentComponent>(entityId) &&
            world.GetComponent<TextboxResidentComponent>(entityId).mTextboxParentEntityId == textboxEntityId
        )
        {
            residentComponents.push_back(entityId);
        }
    }
    
    return residentComponents;
}

char GetCharAtTextboxCoords
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxCol,
    const size_t textboxRow,
    ecs::World& world
)
{
    auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    auto& textboxContent   = textboxComponent.mTextContent;
    
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

    if 
    (
        textboxType != TextboxType::BARE_TEXTBOX && 
        textboxType != TextboxType::CURSORED_BARE_TEXTBOX
    )
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
    
    if 
    (
        textboxType != TextboxType::GENERIC_TEXTBOX &&
        textboxType != TextboxType::BARE_TEXTBOX        
    )
    {
        auto& guiStateComponent = world.GetSingletonComponent<GuiStateSingletonComponent>();

        if (guiStateComponent.mActiveTextboxesStack.size() > 0)
        {
            const auto& previousTextboxComponent= world.GetComponent<TextboxComponent>(guiStateComponent.mActiveTextboxesStack.top());
            if
            (
                previousTextboxComponent.mTextboxType == TextboxType::CURSORED_BARE_TEXTBOX ||
                previousTextboxComponent.mTextboxType == TextboxType::CURSORED_TEXTBOX
            )
            {
                ToggleCursoredTextboxActivityDisplay(guiStateComponent.mActiveTextboxesStack.top(), world);
            }
        }
        

        guiStateComponent.mActiveTextboxesStack.push(textboxEntityId);
    }    
    
    return textboxEntityId;
}

ecs::EntityId CreateChatbox
(
    ecs::World& world,
    const glm::vec3& position /* CHATBOX_POSITION */
)
{
    return CreateTextboxWithDimensions
    (
        TextboxType::CHATBOX,
        CHATBOX_COLS, 
        CHATBOX_ROWS, 
        position.x,
        position.y,
        position.z,
        world
    );
}

ecs::EntityId CreateItemMenu
(
    ecs::World& world,
    const size_t itemCount,
    const int previousCursorRow /* 0 */,
    const int itemOffset /* 0 */
)
{
    const auto itemMenuTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        ITEM_MENU_TEXTBOX_COLS,
        ITEM_MENU_TEXTBOX_ROWS,
        ITEM_MENU_TEXTBOX_POSITION.x,
        ITEM_MENU_TEXTBOX_POSITION.y,
        ITEM_MENU_TEXTBOX_POSITION.z,
        world
    );

    auto cursorComponent = std::make_unique<CursorComponent>();

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = previousCursorRow;

    cursorComponent->mCursorColCount = 1;    
    cursorComponent->mCursorRowCount = math::Min(static_cast<int>(itemCount), 3);

    cursorComponent->mCursorDisplayHorizontalTileOffset = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset = 2;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements = 2;

    WriteCharAtTextboxCoords
    (
        itemMenuTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;

    world.AddComponent<CursorComponent>(itemMenuTextboxEntityId, std::move(cursorComponent));

    auto itemMenuStateComponent = std::make_unique<ItemMenuStateComponent>();
    itemMenuStateComponent->mItemMenuOffsetFromStart = itemOffset;
    world.AddComponent<ItemMenuStateComponent>(itemMenuTextboxEntityId, std::move(itemMenuStateComponent));    

    return itemMenuTextboxEntityId;
}

ecs::EntityId CreatePokeMartMenuTextbox
(
    ecs::World& world
)
{
    const auto martMenuTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        MART_MENU_TEXTBOX_COLS,
        MART_MENU_TEXTBOX_ROWS,
        MART_MENU_TEXTBOX_POSITION.x,
        MART_MENU_TEXTBOX_POSITION.y,
        MART_MENU_TEXTBOX_POSITION.z,
        world
    );

    WriteTextAtTextboxCoords(martMenuTextboxEntityId, "BUY", 2, 1, world);
    WriteTextAtTextboxCoords(martMenuTextboxEntityId, "SELL", 2, 3, world);
    WriteTextAtTextboxCoords(martMenuTextboxEntityId, "QUIT", 2, 5, world);

    auto cursorComponent = std::make_unique<CursorComponent>();

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = 0;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = 3;

    cursorComponent->mCursorDisplayHorizontalTileOffset     = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 1;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;

    WriteCharAtTextboxCoords
    (
        martMenuTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;

    world.AddComponent<CursorComponent>(martMenuTextboxEntityId, std::move(cursorComponent));

    return martMenuTextboxEntityId;
}

ecs::EntityId CreatePokeMartMoneyTextbox
(
    ecs::World& world
)
{
    const auto& playerStateComponent = world.GetSingletonComponent<PlayerStateSingletonComponent>();    

    const auto moneyTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::GENERIC_TEXTBOX,
        MART_MONEY_TEXTBOX_COLS,
        MART_MONEY_TEXTBOX_ROWS,
        MART_MONEY_TEXTBOX_POSITION.x,
        MART_MONEY_TEXTBOX_POSITION.y,
        MART_MONEY_TEXTBOX_POSITION.z,
        world
    );

    const auto& moneyString = "$" + std::to_string(playerStateComponent.mPokeDollarCredits);

    WriteTextAtTextboxCoords(moneyTextboxEntityId, "MONEY", 2, 0, world);
    WriteTextAtTextboxCoords(moneyTextboxEntityId, moneyString, MART_MONEY_TEXTBOX_COLS - 1 - moneyString.size(), 1, world);

    return moneyTextboxEntityId;
}

ecs::EntityId CreatePokeMartItemQuantityTextbox
(
    ecs::World& world,
    const int itemQuantity,
    const int itemPrice    
)
{
    const auto itemQuantityTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CHATBOX,
        MART_ITEM_QUANTITY_TEXTBOX_COLS,
        MART_ITEM_QUANTITY_TEXTBOX_ROWS,
        MART_ITEM_QUANTITY_TEXTBOX_POSITION.x,
        MART_ITEM_QUANTITY_TEXTBOX_POSITION.y,
        MART_ITEM_QUANTITY_TEXTBOX_POSITION.z,
        world
    );

    const auto quantityString = "*" + (itemQuantity < 10 ? "0" + std::to_string(itemQuantity) : std::to_string(itemQuantity));
    const auto itemPriceString = "$" + std::to_string(itemPrice);

    WriteTextAtTextboxCoords(itemQuantityTextboxEntityId, quantityString, 1, 1, world);
    WriteTextAtTextboxCoords(itemQuantityTextboxEntityId, itemPriceString, MART_ITEM_QUANTITY_TEXTBOX_COLS - 1 - itemPriceString.size(), 1, world);

    return itemQuantityTextboxEntityId;
}

ecs::EntityId CreatePCMainOptionsTextbox
(
    ecs::World& world,
    const bool billInteractionCompleted
)
{
    const auto pcMainOptionsTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        PC_MAIN_OPTIONS_TEXTBOX_COLS,
        PC_MAIN_OPTIONS_TEXTBOX_ROWS,
        PC_MAIN_OPTIONS_TEXTBOX_POSITION.x,
        PC_MAIN_OPTIONS_TEXTBOX_POSITION.y,
        PC_MAIN_OPTIONS_TEXTBOX_POSITION.z,
        world
     );
    
    if (billInteractionCompleted)
    {
        WriteTextAtTextboxCoords(pcMainOptionsTextboxEntityId, "BILL's PC", 2, 2, world);
    }
    else
    {
        WriteTextAtTextboxCoords(pcMainOptionsTextboxEntityId, "SOMEONE's PC", 2, 2, world);
    }
    
    WriteTextAtTextboxCoords(pcMainOptionsTextboxEntityId, "PROF.OAK's PC", 2, 4, world);
    WriteTextAtTextboxCoords(pcMainOptionsTextboxEntityId, "LOG OFF", 2, 6, world);
    
    auto cursorComponent = std::make_unique<CursorComponent>();
    
    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = 0;
    
    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = 3;
    
    cursorComponent->mCursorDisplayHorizontalTileOffset     = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 2;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;
    
    WriteCharAtTextboxCoords
    (
        pcMainOptionsTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );
    
    cursorComponent->mWarp = false;
    
    world.AddComponent<CursorComponent>(pcMainOptionsTextboxEntityId, std::move(cursorComponent));
    
    return pcMainOptionsTextboxEntityId;
}

ecs::EntityId CreatePCPokemonSystemOptionsTextbox
(
    ecs::World& world,
    const int cursorRow /* 0 */
)
{
    const auto pcPokemonSystemOptionsTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        PC_POKEMON_SYSTEM_OPTIONS_TEXTBOX_COLS,
        PC_POKEMON_SYSTEM_OPTIONS_TEXTBOX_ROWS,
        PC_POKEMON_SYSTEM_OPTIONS_TEXTBOX_POSITION.x,
        PC_POKEMON_SYSTEM_OPTIONS_TEXTBOX_POSITION.y,
        PC_POKEMON_SYSTEM_OPTIONS_TEXTBOX_POSITION.z,
        world
    );

    WriteTextAtTextboxCoords(pcPokemonSystemOptionsTextboxEntityId, "WITHDRAW <>", 2, 2, world);
    WriteTextAtTextboxCoords(pcPokemonSystemOptionsTextboxEntityId, "DEPOSIT <>", 2, 4, world);
    WriteTextAtTextboxCoords(pcPokemonSystemOptionsTextboxEntityId, "RELEASE <>", 2, 6, world);
    WriteTextAtTextboxCoords(pcPokemonSystemOptionsTextboxEntityId, "SEE YA!", 2, 8, world);
    
    auto cursorComponent = std::make_unique<CursorComponent>();

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = cursorRow;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = 4;

    cursorComponent->mCursorDisplayHorizontalTileOffset     = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 2;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;

    WriteCharAtTextboxCoords
    (
        pcPokemonSystemOptionsTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;

    world.AddComponent<CursorComponent>(pcPokemonSystemOptionsTextboxEntityId, std::move(cursorComponent));
    
    return pcPokemonSystemOptionsTextboxEntityId;
}

ecs::EntityId CreatePCPokemonSystemPokemonListTextbox
(
    ecs::World& world,
    const size_t itemCount,
    const int previousCursorRow /* 0 */,
    const int itemOffset /* 0 */
)
{
    const auto pcPokemonSystemPokemonListTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        PC_POKEMON_SYSTEM_POKEMON_LIST_TEXTBOX_COLS,
        PC_POKEMON_SYSTEM_POKEMON_LIST_TEXTBOX_ROWS,
        PC_POKEMON_SYSTEM_POKEMON_LIST_TEXTBOX_POSITION.x,
        PC_POKEMON_SYSTEM_POKEMON_LIST_TEXTBOX_POSITION.y,
        PC_POKEMON_SYSTEM_POKEMON_LIST_TEXTBOX_POSITION.z,
        world
    );

    auto cursorComponent = std::make_unique<CursorComponent>();

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = previousCursorRow;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = math::Min(static_cast<int>(itemCount), 3);

    cursorComponent->mCursorDisplayHorizontalTileOffset     = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 2;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;

    WriteCharAtTextboxCoords
    (
        pcPokemonSystemPokemonListTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;

    world.AddComponent<CursorComponent>(pcPokemonSystemPokemonListTextboxEntityId, std::move(cursorComponent));

    auto itemMenuStateComponent = std::make_unique<ItemMenuStateComponent>();
    itemMenuStateComponent->mItemMenuOffsetFromStart = itemOffset;
    world.AddComponent<ItemMenuStateComponent>(pcPokemonSystemPokemonListTextboxEntityId, std::move(itemMenuStateComponent));
    
    return pcPokemonSystemPokemonListTextboxEntityId;
}

ecs::EntityId CreatePCPokemonSelectedOptionsTextbox
(
    ecs::World& world,
    const PokemonSystemOperationType operationType,
    const int cursorRow /* 0 */
)
{
    const auto pcPokemonSelectedOptionsTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        PC_POKEMON_SELECTED_OPTIONS_TEXTBOX_COLS,
        PC_POKEMON_SELECTED_OPTIONS_TEXTBOX_ROWS,
        PC_POKEMON_SELECTED_OPTIONS_TEXTBOX_POSITION.x,
        PC_POKEMON_SELECTED_OPTIONS_TEXTBOX_POSITION.y,
        PC_POKEMON_SELECTED_OPTIONS_TEXTBOX_POSITION.z,
        world
    );    

    WriteTextAtTextboxCoords(pcPokemonSelectedOptionsTextboxEntityId, operationType == PokemonSystemOperationType::DEPOSIT ? "DEPOSIT" : "WITHDRAW", 2, 2, world);
    WriteTextAtTextboxCoords(pcPokemonSelectedOptionsTextboxEntityId, "STATS", 2, 4, world);
    WriteTextAtTextboxCoords(pcPokemonSelectedOptionsTextboxEntityId, "CANCEL", 2, 6, world);    

    auto cursorComponent = std::make_unique<CursorComponent>();

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = cursorRow;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = 3;

    cursorComponent->mCursorDisplayHorizontalTileOffset     = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 2;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;

    WriteCharAtTextboxCoords
    (
        pcPokemonSelectedOptionsTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;

    world.AddComponent<CursorComponent>(pcPokemonSelectedOptionsTextboxEntityId, std::move(cursorComponent));

    return pcPokemonSelectedOptionsTextboxEntityId;
}

ecs::EntityId CreatePokedexMainViewInvisibleListTextbox
(    
    ecs::World& world,
    const int previousCursorRow /* 0 */,
    const int itemOffset /* 0 */
)
{
    const auto pokedexMainViewInvisibleListTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_BARE_TEXTBOX,
        POKEDEX_MAIN_VIEW_INVISIBLE_LIST_TEXTBOX_COLS,
        POKEDEX_MAIN_VIEW_INVISIBLE_LIST_TEXTBOX_ROWS,
        POKEDEX_MAIN_VIEW_INVISIBLE_LIST_TEXTBOX_POSITION.x,
        POKEDEX_MAIN_VIEW_INVISIBLE_LIST_TEXTBOX_POSITION.y,
        POKEDEX_MAIN_VIEW_INVISIBLE_LIST_TEXTBOX_POSITION.z,
        world
    );

    auto cursorComponent = std::make_unique<CursorComponent>();

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = previousCursorRow;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = POKEDEX_MAIN_VIEW_INVISIBLE_LIST_TEXTBOX_ROWS/2;

    cursorComponent->mCursorDisplayHorizontalTileOffset     = 0;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 1;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;

    WriteCharAtTextboxCoords
    (
        pokedexMainViewInvisibleListTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;

    world.AddComponent<CursorComponent>(pokedexMainViewInvisibleListTextboxEntityId, std::move(cursorComponent));

    auto itemMenuStateComponent = std::make_unique<ItemMenuStateComponent>();
    itemMenuStateComponent->mItemMenuOffsetFromStart = itemOffset;
    world.AddComponent<ItemMenuStateComponent>(pokedexMainViewInvisibleListTextboxEntityId, std::move(itemMenuStateComponent));

    return pokedexMainViewInvisibleListTextboxEntityId;
}

ecs::EntityId CreateBlackboardTextbox
(
    ecs::World& world,
    const int initialCursorCol /* 0 */,
    const int initialCursorRow /* 0 */
)
{
    const auto blackboardTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        BLACKBOARD_TEXTBOX_COLS,
        BLACKBOARD_TEXTBOX_ROWS,
        BLACKBOARD_TEXTBOX_POSITION.x,
        BLACKBOARD_TEXTBOX_POSITION.y,
        BLACKBOARD_TEXTBOX_POSITION.z,
        world
    );

    WriteTextAtTextboxCoords(blackboardTextboxEntityId, "SLP", 2, 2, world);
    WriteTextAtTextboxCoords(blackboardTextboxEntityId, "BRN", 7, 2, world);
    WriteTextAtTextboxCoords(blackboardTextboxEntityId, "PSN", 2, 4, world);
    WriteTextAtTextboxCoords(blackboardTextboxEntityId, "FRZ", 7, 4, world);
    WriteTextAtTextboxCoords(blackboardTextboxEntityId, "PAR", 2, 6, world);
    WriteTextAtTextboxCoords(blackboardTextboxEntityId, "QUIT", 7, 6, world);

    auto cursorComponent = std::make_unique<CursorComponent>();

    cursorComponent->mCursorCol = initialCursorCol;
    cursorComponent->mCursorRow = initialCursorRow;

    cursorComponent->mCursorColCount = 2;
    cursorComponent->mCursorRowCount = 3;

    cursorComponent->mCursorDisplayHorizontalTileOffset = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset = 2;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 5;
    cursorComponent->mCursorDisplayVerticalTileIncrements = 2;

    WriteCharAtTextboxCoords
    (
        blackboardTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;

    world.AddComponent<CursorComponent>(blackboardTextboxEntityId, std::move(cursorComponent));

    return blackboardTextboxEntityId;
}

ecs::EntityId CreateYesNoTextbox
(
    ecs::World& world,
    const glm::vec3& position
)
{
    const auto yesNoTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        YES_NO_TEXTBOX_COLS,
        YES_NO_TEXTBOX_ROWS,
        position.x,
        position.y,
        position.z,
        world
    );

    WriteTextAtTextboxCoords(yesNoTextboxEntityId, "YES", 2, 1, world);
    WriteTextAtTextboxCoords(yesNoTextboxEntityId, "NO", 2, 3, world);

    auto cursorComponent = std::make_unique<CursorComponent>();  

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = 0;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = 2;

    cursorComponent->mCursorDisplayHorizontalTileOffset = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset = 1;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements = 2;

    WriteCharAtTextboxCoords
    (
        yesNoTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;

    world.AddComponent<CursorComponent>(yesNoTextboxEntityId, std::move(cursorComponent));

    return yesNoTextboxEntityId;
}

ecs::EntityId CreateHealCancelTextbox
(
    ecs::World& world,
    const glm::vec3& position
)
{
    const auto healCancelTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        HEAL_CANCEL_TEXTBOX_COLS,
        HEAL_CANCEL_TEXTBOX_ROWS,
        position.x,
        position.y,
        position.z,
        world
    );

    WriteTextAtTextboxCoords(healCancelTextboxEntityId, "HEAL", 2, 2, world);
    WriteTextAtTextboxCoords(healCancelTextboxEntityId, "CANCEL", 2, 4, world);

    auto cursorComponent = std::make_unique<CursorComponent>();

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = 0;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = 2;

    cursorComponent->mCursorDisplayHorizontalTileOffset = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset = 2;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements = 2;

    WriteCharAtTextboxCoords
    (
        healCancelTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;

    world.AddComponent<CursorComponent>(healCancelTextboxEntityId, std::move(cursorComponent));

    return healCancelTextboxEntityId;
}

ecs::EntityId CreateUseTossTextbox
(
    ecs::World& world,
    const glm::vec3& position
)
{
    const auto useTossTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        USE_TOSS_TEXTBOX_COLS,
        USE_TOSS_TEXTBOX_ROWS,
        position.x,
        position.y,
        position.z,
        world
    );

    WriteTextAtTextboxCoords(useTossTextboxEntityId, "USE", 2, 1, world);
    WriteTextAtTextboxCoords(useTossTextboxEntityId, "TOSS", 2, 3, world);

    auto cursorComponent = std::make_unique<CursorComponent>();

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = 0;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = 2;

    cursorComponent->mCursorDisplayHorizontalTileOffset     = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 1;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;

    WriteCharAtTextboxCoords
    (
        useTossTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;

    world.AddComponent<CursorComponent>(useTossTextboxEntityId, std::move(cursorComponent));

    return useTossTextboxEntityId;
}

ecs::EntityId CreateOverworldMainMenuTextbox
(
    ecs::World& world,
    const bool hasPokedex,
    const int lastSelectedMenuItemRow /* 0 */
)
{
    const auto overworldMenuTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        OVERWORLD_MAIN_MENU_TEXTBOX_COLS_WITH_POKEDEX,
        hasPokedex ? OVERWORLD_MAIN_MENU_TEXTBOX_ROWS_WITH_POKEDEX : OVERWORLD_MAIN_MENU_TEXTBOX_ROWS_WITH_POKEDEX - 2,
        hasPokedex ? OVERWORLD_MAIN_MENU_TEXTBOX_POSITION_WITH_POKEDEX.x : OVERWORLD_MAIN_MENU_TEXTBOX_POSITION_WITHOUT_POKEDEX.x,
        hasPokedex ? OVERWORLD_MAIN_MENU_TEXTBOX_POSITION_WITH_POKEDEX.y : OVERWORLD_MAIN_MENU_TEXTBOX_POSITION_WITHOUT_POKEDEX.y,
        hasPokedex ? OVERWORLD_MAIN_MENU_TEXTBOX_POSITION_WITH_POKEDEX.z : OVERWORLD_MAIN_MENU_TEXTBOX_POSITION_WITHOUT_POKEDEX.z,
        world
    );

    const auto& playerStateComponent = world.GetSingletonComponent<PlayerStateSingletonComponent>();

    if (hasPokedex)
    {
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, "POK^DEX", 2, 2, world);
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, "POK^MON", 2, 4, world);
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, "ITEM", 2, 6, world);
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, playerStateComponent.mPlayerTrainerName.GetString(), 2, 8, world);
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, "SAVE", 2, 10, world);
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, "OPTION", 2, 12, world);
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, "EXIT", 2, 14, world);
    }
    else
    {
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, "POK^MON", 2, 2, world);
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, "ITEM", 2, 4, world);
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, playerStateComponent.mPlayerTrainerName.GetString(), 2, 6, world);
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, "SAVE", 2, 8, world);
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, "OPTION", 2, 10, world);
        WriteTextAtTextboxCoords(overworldMenuTextboxEntityId, "EXIT", 2, 12, world);
    }
    

    auto cursorComponent = std::make_unique<CursorComponent>();

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = lastSelectedMenuItemRow;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = hasPokedex ? 7 : 6;

    cursorComponent->mCursorDisplayHorizontalTileOffset     = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 2;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;

    WriteCharAtTextboxCoords
    (
        overworldMenuTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;

    world.AddComponent<CursorComponent>(overworldMenuTextboxEntityId, std::move(cursorComponent));

    return overworldMenuTextboxEntityId;
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
    ecs::World& world,
    const glm::vec3& position /* ENCOUNTER_FIGHT_MENU_TEXTBOX_POSITION */
)
{
    const auto fightMenuTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        ENCOUNTER_FIGHT_MENU_TEXTBOX_COLS,
        ENCOUNTER_FIGHT_MENU_TEXTBOX_ROWS,
        position.x,
        position.y,
        position.z,
        world
    );

    auto numberOfValidMoves = 0;
    for (auto i = 0U; i < moveset.size(); ++i)
    {
        if (moveset[i] != nullptr)
        {
            WriteTextAtTextboxCoords(fightMenuTextboxEntityId, moveset[i]->mName.GetString(), 2, i + 1, world);
            numberOfValidMoves++;
        }
        else
        {
            WriteTextAtTextboxCoords(fightMenuTextboxEntityId, "-", 2, i + 1, world);
        }        
    }    

    auto cursorComponent = std::make_unique<CursorComponent>();    
    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = lastSelectedMoveIndex;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = numberOfValidMoves;

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

ecs::EntityId CreateSaveScreenPlayerStatsTextbox
(    
    ecs::World& world
)
{
    const auto playerStatsTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::GENERIC_TEXTBOX,
        SAVE_SCREEN_PLAYER_STATS_TEXTBOX_COLS, 
        SAVE_SCREEN_PLAYER_STATS_TEXTBOX_ROWS, 
        SAVE_SCREEN_PLAYER_STATS_TEXTBOX_POSITION.x,
        SAVE_SCREEN_PLAYER_STATS_TEXTBOX_POSITION.y,
        SAVE_SCREEN_PLAYER_STATS_TEXTBOX_POSITION.z,
        world
    );

    const auto& playerStateComponent = world.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto playerName            = playerStateComponent.mPlayerTrainerName.GetString();
    const auto playerBadgesString    = std::to_string(GetNumberOfBadges(world));
    const auto numPokemonOwnedString = std::to_string(GetNumberOfPokemonWithPokedexEntryType(PokedexEntryType::OWNED, world));
    const auto timePlayedString      = GetHoursMinutesStringFromSeconds(playerStateComponent.mSecondsPlayed);

    WriteTextAtTextboxCoords(playerStatsTextboxEntityId, "PLAYER", 1, 2, world);
    WriteTextAtTextboxCoords(playerStatsTextboxEntityId, "BADGES", 1, 4, world);
    WriteTextAtTextboxCoords(playerStatsTextboxEntityId, "POK^DEX", 1, 6, world);
    WriteTextAtTextboxCoords(playerStatsTextboxEntityId, "TIME", 1, 8, world);
    WriteTextAtTextboxCoords(playerStatsTextboxEntityId, playerName, 15 - playerName.size(), 2, world);
    WriteTextAtTextboxCoords(playerStatsTextboxEntityId, playerBadgesString, 15 - playerBadgesString.size(), 4, world);
    WriteTextAtTextboxCoords(playerStatsTextboxEntityId, numPokemonOwnedString, 15 - numPokemonOwnedString.size(), 6, world);
    WriteTextAtTextboxCoords(playerStatsTextboxEntityId, timePlayedString, 15 - timePlayedString.size(), 8, world);
        
    return playerStatsTextboxEntityId;
}

ecs::EntityId CreateEncounterFightMenuMoveInfoTextbox
(
    const PokemonMoveStats& pokemonMove,
    ecs::World& world
)
{
    const auto fightMenuMoveInfoTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::GENERIC_TEXTBOX,
        ENCOUNTER_FIGHT_MENU_MOVE_INFO_TEXTBOX_COLS,
        ENCOUNTER_FIGHT_MENU_MOVE_INFO_TEXTBOX_ROWS,
        ENCOUNTER_FIGHT_MENU_MOVE_INFO_TEXTBOX_POSITION.x,
        ENCOUNTER_FIGHT_MENU_MOVE_INFO_TEXTBOX_POSITION.y,
        ENCOUNTER_FIGHT_MENU_MOVE_INFO_TEXTBOX_POSITION.z,
        world
    );

    WriteTextAtTextboxCoords(fightMenuMoveInfoTextboxEntityId, "TYPE/", 1, 1, world);
    WriteTextAtTextboxCoords(fightMenuMoveInfoTextboxEntityId, pokemonMove.mType.GetString(), 2, 2, world);
    WriteTextAtTextboxCoords(fightMenuMoveInfoTextboxEntityId, "/", 7, 3, world);

    const auto powerPointsLeftString  = std::to_string(pokemonMove.mPowerPointsLeft);
    const auto totalPowerPointsString = std::to_string(pokemonMove.mTotalPowerPoints);

    WriteTextAtTextboxCoords(fightMenuMoveInfoTextboxEntityId, powerPointsLeftString, 7 - powerPointsLeftString.size(), 3, world);
    WriteTextAtTextboxCoords(fightMenuMoveInfoTextboxEntityId, totalPowerPointsString, 10 - totalPowerPointsString.size(), 3, world);

    return fightMenuMoveInfoTextboxEntityId;
}

ecs::EntityId CreatePokemonMiniStatsDisplay
(
    const Pokemon& pokemon,
    const glm::vec3& position,
    ecs::World& world
)
{
    const auto pokemonStatsDisplayEntityId = CreateTextboxWithDimensions
    (
        TextboxType::GENERIC_TEXTBOX,
        POKEMON_MINI_STATS_DISPLAY_TEXTBOX_COLS,
        POKEMON_MINI_STATS_DISPLAY_TEXTBOX_ROWS,
        position.x,
        position.y,
        position.z,
        world
    );

    const auto attackStatString  = std::to_string(pokemon.mAttack);
    const auto defenseStatString = std::to_string(pokemon.mDefense);
    const auto speedStatString   = std::to_string(pokemon.mSpeed);
    const auto specialStatString = std::to_string(pokemon.mSpecial);

    WriteTextAtTextboxCoords(pokemonStatsDisplayEntityId, "ATTACK", 2, 1, world);
    WriteTextAtTextboxCoords(pokemonStatsDisplayEntityId, attackStatString, 9 - attackStatString.size(), 2, world);
    WriteTextAtTextboxCoords(pokemonStatsDisplayEntityId, "DEFENSE", 2, 3, world);
    WriteTextAtTextboxCoords(pokemonStatsDisplayEntityId, defenseStatString, 9 - defenseStatString.size(), 4, world);
    WriteTextAtTextboxCoords(pokemonStatsDisplayEntityId, "SPEED", 2, 5, world);
    WriteTextAtTextboxCoords(pokemonStatsDisplayEntityId, speedStatString, 9 - speedStatString.size(), 6, world);
    WriteTextAtTextboxCoords(pokemonStatsDisplayEntityId, "SPECIAL", 2, 7, world);
    WriteTextAtTextboxCoords(pokemonStatsDisplayEntityId, specialStatString, 9 - specialStatString.size(), 8, world);

    return pokemonStatsDisplayEntityId;
}

ecs::EntityId CreatePokemonStatsDisplayTextbox
(
    ecs::World& world
)
{
    return CreateTextboxWithDimensions
    (
        TextboxType::BARE_TEXTBOX, 
        POKEMON_STATS_DISPLAY_TEXTBOX_COLS, 
        POKEMON_STATS_DISPLAY_TEXTBOX_ROWS,
        POKEMON_STATS_DISPLAY_TEXTBOX_POSITION.x,
        POKEMON_STATS_DISPLAY_TEXTBOX_POSITION.y,
        POKEMON_STATS_DISPLAY_TEXTBOX_POSITION.z,
        world
    );    
}

ecs::EntityId CreatePokedexPokemonEntryDisplayTextbox
(
    ecs::World& world
)
{
    return CreateTextboxWithDimensions
    (
        TextboxType::BARE_TEXTBOX,
        POKEDEX_POKEMON_ENTRY_DISPLAY_TEXTBOX_COLS,
        POKEDEX_POKEMON_ENTRY_DISPLAY_TEXTBOX_ROWS,
        POKEDEX_POKEMON_ENTRY_DISPLAY_TEXTBOX_POSITION.x,
        POKEDEX_POKEMON_ENTRY_DISPLAY_TEXTBOX_POSITION.y,
        POKEDEX_POKEMON_ENTRY_DISPLAY_TEXTBOX_POSITION.z,
        world
    );
}

ecs::EntityId CreateNameSelectionCharactersEnclosingTextbox
(
    ecs::World& world
)
{
    return CreateTextboxWithDimensions
    (
        TextboxType::GENERIC_TEXTBOX,
        NAME_SELECTION_CHARACTERS_ENCLOSING_TEXTBOX_COLS,
        NAME_SELECTION_CHARACTERS_ENCLOSING_TEXTBOX_ROWS,
        NAME_SELECTION_CHARACTERS_ENCLOSING_TEXTBOX_POSITION.x,
        NAME_SELECTION_CHARACTERS_ENCLOSING_TEXTBOX_POSITION.y,
        NAME_SELECTION_CHARACTERS_ENCLOSING_TEXTBOX_POSITION.z,
        world
    );
}

ecs::EntityId CreateNameSelectionCharactersInvisibleTextbox
(
    const bool uppercaseMode,
    ecs::World& world,
    const int previousCursorCol /* 0 */,
    const int previousCursorRow /* 0 */
)
{
    const auto nameSelectionCharactersTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_BARE_TEXTBOX,
        NAME_SELECTION_CHARACTERS_INVISIBLE_TEXTBOX_COLS,
        NAME_SELECTION_CHARACTERS_INVISIBLE_TEXTBOX_ROWS,
        NAME_SELECTION_CHARACTERS_INVISIBLE_TEXTBOX_POSITION.x,
        NAME_SELECTION_CHARACTERS_INVISIBLE_TEXTBOX_POSITION.y,
        NAME_SELECTION_CHARACTERS_INVISIBLE_TEXTBOX_POSITION.z,
        world
    );
    
    if (uppercaseMode)
    {
        WriteTextAtTextboxCoords(nameSelectionCharactersTextboxEntityId, "A B C D E F G H I", 1, 0, world);
        WriteTextAtTextboxCoords(nameSelectionCharactersTextboxEntityId, "J K L M N O P Q R", 1, 2, world);
        WriteTextAtTextboxCoords(nameSelectionCharactersTextboxEntityId, "S T U V W X Y Z  ", 1, 4, world);
        WriteTextAtTextboxCoords(nameSelectionCharactersTextboxEntityId, "lower case", 1, 10, world);
    }
    else
    {
        WriteTextAtTextboxCoords(nameSelectionCharactersTextboxEntityId, "a b c d e f g h i", 1, 0, world);
        WriteTextAtTextboxCoords(nameSelectionCharactersTextboxEntityId, "j k l m n o p q r", 1, 2, world);
        WriteTextAtTextboxCoords(nameSelectionCharactersTextboxEntityId, "s t u v w x y z  ", 1, 4, world);
        WriteTextAtTextboxCoords(nameSelectionCharactersTextboxEntityId, "UPPER CASE", 1, 10, world);
    }
    
    WriteTextAtTextboxCoords(nameSelectionCharactersTextboxEntityId, "* ( ) : ; # @ < >", 1, 6, world);
    WriteTextAtTextboxCoords(nameSelectionCharactersTextboxEntityId, "- ? ! [ ] / . , %", 1, 8, world);
    
    auto cursorComponent = std::make_unique<CursorComponent>();
    cursorComponent->mCursorCol = previousCursorCol;
    cursorComponent->mCursorRow = previousCursorRow;
    
    cursorComponent->mCursorColCount = NAME_SELECTION_CHARACTERS_INVISIBLE_TEXTBOX_COLS/2;
    cursorComponent->mCursorRowCount = NAME_SELECTION_CHARACTERS_INVISIBLE_TEXTBOX_ROWS/2 + 1;
    
    cursorComponent->mCursorDisplayHorizontalTileOffset = 0;
    cursorComponent->mCursorDisplayVerticalTileOffset   = 0;
    
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 2;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;
    
    WriteCharAtTextboxCoords
    (
        nameSelectionCharactersTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );
    
    cursorComponent->mWarp = true;
    cursorComponent->mNameSelectionSpecialCase = true;

    world.AddComponent<CursorComponent>(nameSelectionCharactersTextboxEntityId, std::move(cursorComponent));
    
    return nameSelectionCharactersTextboxEntityId;
}

ecs::EntityId CreateNameSelectionTitleInvisibleTextbox
(
    ecs::World& world
)
{
    return  CreateTextboxWithDimensions
    (
        TextboxType::BARE_TEXTBOX,
        NAME_SELECTION_TITLE_INVISIBLE_TEXTBOX_COLS,
        NAME_SELECTION_TITLE_INVISIBLE_TEXTBOX_ROWS,
        NAME_SELECTION_TITLE_INVISIBLE_TEXTBOX_POSITION.x,
        NAME_SELECTION_TITLE_INVISIBLE_TEXTBOX_POSITION.y,
        NAME_SELECTION_TITLE_INVISIBLE_TEXTBOX_POSITION.z,
        world
    );
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
    
    const auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    if (textboxComponent.mTextboxType == TextboxType::CHATBOX)
    {
        guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::NORMAL;
        guiStateComponent.mActiveChatboxContentState = ChatboxContentEndState::NORMAL;
    }
    
    const auto& entityIds = world.GetActiveEntities();
    for (const auto& entityId: entityIds)
    {
        if
        (
            world.HasComponent<TextboxResidentComponent>(entityId) &&
            world.GetComponent<TextboxResidentComponent>(entityId).mTextboxParentEntityId == textboxEntityId
        )
        {
            world.DestroyEntity(entityId);
        }
    }
    
    world.DestroyEntity(textboxEntityId);

    if (guiStateComponent.mActiveTextboxesStack.size() > 0)
    {
        const auto& previousTextboxComponent = world.GetComponent<TextboxComponent>(guiStateComponent.mActiveTextboxesStack.top());
        if
        (
            previousTextboxComponent.mTextboxType == TextboxType::CURSORED_BARE_TEXTBOX ||
            previousTextboxComponent.mTextboxType == TextboxType::CURSORED_TEXTBOX
        )
        {
            ToggleCursoredTextboxActivityDisplay(guiStateComponent.mActiveTextboxesStack.top(), world);
        }
    }
}

void DestroyGenericOrBareTextbox
(
    ecs::EntityId textboxEntityId,
    ecs::World& world
)
{
    auto& guiStateComponent  = world.GetSingletonComponent<GuiStateSingletonComponent>();
    auto activeTextboxesCopy = guiStateComponent.mActiveTextboxesStack;
    
    // Make sure the textbox is not in the active textboxes stack
    while (activeTextboxesCopy.size() != 0)
    {
        if (activeTextboxesCopy.top() == textboxEntityId)
        {
            assert(false && "Tried to destroy a non-generic/non-bare textbox");
        }
        activeTextboxesCopy.pop();
    }

    const auto& entityIds = world.GetActiveEntities();
    for (const auto& entityId : entityIds)
    {
        if
        (
            world.HasComponent<TextboxResidentComponent>(entityId) &&
            world.GetComponent<TextboxResidentComponent>(entityId).mTextboxParentEntityId == textboxEntityId
        )
        {
            world.DestroyEntity(entityId);
        }
    }

    world.DestroyEntity(textboxEntityId);
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
        // In case a previous glyph is present and has a renderable component, instead of going 
        // throught the whole renderable creation process, swap only the necessary data to change to the new character
        if (world.HasComponent<RenderableComponent>(textboxContent[textboxRow][textboxCol].mEntityId))
        {
            textboxContent[textboxRow][textboxCol].mCharacter = character;
            auto& renderableComponent = world.GetComponent<RenderableComponent>(textboxContent[textboxRow][textboxCol].mEntityId);
            renderableComponent.mAnimationsToMeshes[renderableComponent.mActiveAnimationNameId][0] = guiStateComponent.mFontEntities.at(character);
            return;
        }
        else
        {
            world.DestroyEntity(textboxContent[textboxRow][textboxCol].mEntityId);
        }
    }
    
    const auto characterEntityId = world.CreateEntity();
    
    textboxComponent.mTextContent[textboxRow][textboxCol].mCharacter = character;
    textboxComponent.mTextContent[textboxRow][textboxCol].mEntityId  = characterEntityId;
    
    auto textboxResidentComponent                    = std::make_unique<TextboxResidentComponent>();
    textboxResidentComponent->mTextboxParentEntityId = textboxEntityId;
    
    world.AddComponent<TextboxResidentComponent>(characterEntityId, std::move(textboxResidentComponent));
    
    // Don't add transform or model components for whitespace character
    if (character == ' ' || character == '_')
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
        textboxTransformComponent.mPosition.z -0.06f
     );
    
    world.AddComponent<RenderableComponent>(characterEntityId, std::move(renderableComponent));
    world.AddComponent<TransformComponent>(characterEntityId, std::move(transformComponent));
}

void WriteSpecialCharacterAtTextboxCoords
(
    const ecs::EntityId textboxEntityId,
    const SpecialCharacter specialCharacter,
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
    
    const auto& specialCharacterAtlasCoords = SPECIAL_CHARACTERS_TO_ATLAS_COORDS.at(specialCharacter);

    if (textboxContent[textboxRow][textboxCol].mEntityId != ecs::NULL_ENTITY_ID)
    {
        world.DestroyEntity(textboxContent[textboxRow][textboxCol].mEntityId);
    }
    
    const auto characterEntityId = world.CreateEntity();
    
    textboxComponent.mTextContent[textboxRow][textboxCol].mCharacter = '_';
    textboxComponent.mTextContent[textboxRow][textboxCol].mEntityId  = characterEntityId;
    
    auto textboxResidentComponent                    = std::make_unique<TextboxResidentComponent>();
    textboxResidentComponent->mTextboxParentEntityId = textboxEntityId;
    
    world.AddComponent<TextboxResidentComponent>(characterEntityId, std::move(textboxResidentComponent));
   
    auto renderableComponent                    = std::make_unique<RenderableComponent>();
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + "gui.png");
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;
    renderableComponent->mAnimationsToMeshes[renderableComponent->mActiveAnimationNameId].push_back
    (
        LoadMeshFromAtlasTexCoords
        (
            specialCharacterAtlasCoords.mCol,
            specialCharacterAtlasCoords.mRow,
            GUI_ATLAS_COLS,
            GUI_ATLAS_ROWS,
            false,
            GUI_COMPONENTS_MODEL_NAME
        )
    );
    
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
     textboxTransformComponent.mPosition.z -0.06f
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

    assert((textboxCol + text.size() - 1 < textboxContent[textboxRow].size()) && "Word cannot fit in specified textbox coords");
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

void QueueDialogForChatbox
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
    
    world.DestroyEntity(characterEntityId);
}

void DeleteTextAtTextboxRow
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxRow,
    ecs::World& world
)
{
    auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    
    for (size_t textboxCol = 0U; textboxCol < textboxComponent.mTextContent[textboxRow].size(); ++textboxCol)
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

void ToggleCursoredTextboxActivityDisplay
(
    const ecs::EntityId textboxEntityId,
    ecs::World& world
)
{
    const auto& textboxComponent = world.GetComponent<TextboxComponent>(textboxEntityId);
    
    for (auto row = 0; row < textboxComponent.mTextboxTileRows; ++row)
    {
        for (auto col = 0; col < textboxComponent.mTextboxTileCols; ++col)
        {
            const auto currentChar = GetCharAtTextboxCoords(textboxEntityId, col, row, world);
            if (currentChar == '{')
            {
                WriteCharAtTextboxCoords(textboxEntityId, '}', col, row, world);
            }
            else if (currentChar == '}')
            {
                WriteCharAtTextboxCoords(textboxEntityId, '{', col, row, world);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
