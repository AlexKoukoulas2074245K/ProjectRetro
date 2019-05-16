//
//  TextboxUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 05/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef TextboxUtils_h
#define TextboxUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../components/TextboxComponent.h"

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ecs::EntityId GetActiveTextboxEntityId
(
    const ecs::World& world
);

size_t GetFirstEmptyColumnInTextboxRow
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxRow,
    const ecs::World& world
);

const std::vector<TextboxCharacterEntry>& GetTextboxRowContent
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxRow,
    ecs::World& world
);

char GetCharacterAtTextboxCoords
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxCol,
    const size_t textboxRow,
    ecs::World& world
);

ecs::EntityId CreateTextboxWithDimensions
(
    const int textboxTileCols,
    const int textboxTileRows,
    const float textboxOriginX,
    const float textboxOriginY,
    ecs::World& world
);

void DestroyActiveTextbox
(
    ecs::World& world
);

void WriteCharAtTextboxCoords
(
    const ecs::EntityId textboxEntityId,
    const char character,
    const size_t textboxCol,
    const size_t textboxRow,
    ecs::World& world
);

void WriteTextAtTextboxCoords
(
    const ecs::EntityId textboxEntityId,
    const std::string& text,
    const size_t textboxCol,
    const size_t textboxRow,
    ecs::World& world
);

void QueueDialogForTextbox
(
    const ecs::EntityId textboxEntityId,
    const std::string& rawDialogText,
    ecs::World& world
);

void DeleteCharAtTextboxCoords
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxCol,
    const size_t textboxRow,
    ecs::World& world
);

void DeleteTextAtTextboxRow
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxRow,
    ecs::World& world
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TextboxUtils_h */