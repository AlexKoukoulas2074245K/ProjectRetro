//
//  PokemonSelectionViewSpriteUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/07/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonSelectionViewSpriteUtils.h"
#include "TextboxUtils.h"
#include "MathUtils.h"
#include "../components/CursorComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const glm::vec3 POKEMON_SELECTED_TEXTBOX_POSITION = glm::vec3(0.375f, -0.6201f, -0.2f);

static const int POKEMON_SELECTED_TEXTBOX_COLS = 9;
static const int POKEMON_SELECTED_TEXTBOX_ROWS = 7;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId CreatePokemonSelectionViewSelectionTextbox
(
    bool isInEncounter,
    ecs::World& world
)
{
    const auto textboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        POKEMON_SELECTED_TEXTBOX_COLS,
        POKEMON_SELECTED_TEXTBOX_ROWS,
        POKEMON_SELECTED_TEXTBOX_POSITION.x,
        POKEMON_SELECTED_TEXTBOX_POSITION.y,
        POKEMON_SELECTED_TEXTBOX_POSITION.z,
        world
    );

    if (isInEncounter)
    {
        WriteTextAtTextboxCoords(textboxEntityId, "SWITCH", 2, 1, world);
        WriteTextAtTextboxCoords(textboxEntityId, "STATS", 2, 3, world);
    }
    else
    {
        WriteTextAtTextboxCoords(textboxEntityId, "STATS", 2, 1, world);
        WriteTextAtTextboxCoords(textboxEntityId, "SWITCH", 2, 3, world);
    }
    WriteTextAtTextboxCoords(textboxEntityId, "CANCEL", 2, 5, world);

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
        textboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        world
    );

    cursorComponent->mWarp = false;
    
    world.AddComponent<CursorComponent>(textboxEntityId, std::move(cursorComponent));

    return textboxEntityId;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

