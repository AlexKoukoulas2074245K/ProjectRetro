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

#include "MathUtils.h"
#include "../components/TextboxComponent.h"
#include "../../ECS.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct Pokemon;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

extern const glm::vec3 CHATBOX_POSITION; 
extern const glm::vec3 ENCOUNTER_FIGHT_MENU_TEXTBOX_POSITION;

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

std::string GetTextboxRowString
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxRow,
    ecs::World& world
);

std::vector<ecs::EntityId> GetAllTextboxResidentComponents
(
    const ecs::EntityId textboxEntityId,
    ecs::World& world
);

char GetCharAtTextboxCoords
(
    const ecs::EntityId textboxEntityId,
    const size_t textboxCol,
    const size_t textboxRow,
    ecs::World& world
);

ecs::EntityId CreateTextboxWithDimensions
(
    const TextboxType textboxType,
    const int textboxTileCols,
    const int textboxTileRows,
    const float textboxOriginX,
    const float textboxOriginY,
    const float textboxZ,
    ecs::World& world
);

ecs::EntityId CreateChatbox
(
    ecs::World& world,
    const glm::vec3& position = CHATBOX_POSITION
);

ecs::EntityId CreateItemMenu
(
    ecs::World& world,
    const size_t itemCount,
    const int previousCursorRow = 0,
    const int itemOffset = 0
);

ecs::EntityId CreatePokeMartMenuTextbox
(
    ecs::World& world
);

ecs::EntityId CreatePokeMartMoneyTextbox
(
    ecs::World& world
);

ecs::EntityId CreateBlackboardTextbox
(
    ecs::World& world,
    const int initialCursorCol = 0,
    const int initialCursorRow = 0
);

ecs::EntityId CreateYesNoTextbox
(
    ecs::World& world,
    const glm::vec3& position
);

ecs::EntityId CreateHealCancelTextbox
(
    ecs::World& world,
    const glm::vec3& position
);

ecs::EntityId CreateUseTossTextbox
(
    ecs::World& world,
    const glm::vec3& position
);

ecs::EntityId CreateOverworldMainMenuTextbox
(
    ecs::World& world,
    const bool hasPokedex,
    const int lastSelectedMenuItemRow = 0
);

ecs::EntityId CreateEncounterMainMenuTextbox
(
    const MainMenuActionType actionTypeSelected,
    ecs::World& world
);

ecs::EntityId CreateEncounterFightMenuTextbox
(
    const PokemonMoveSet& moveset,
    const int lastSelectedMoveIndex,
    ecs::World& world,
    const glm::vec3& position = ENCOUNTER_FIGHT_MENU_TEXTBOX_POSITION
);

ecs::EntityId CreateEncounterFightMenuMoveInfoTextbox
(
    const PokemonMoveStats& pokemonMove,
    ecs::World& world
);

ecs::EntityId CreateSaveScreenPlayerStatsTextbox
(    
    ecs::World& world
);

ecs::EntityId CreatePokemonMiniStatsDisplay
(
    const Pokemon& pokemon,
    const glm::vec3& position,
    ecs::World& world
);

ecs::EntityId CreatePokemonStatsDisplayTextbox
(
    ecs::World& world
);

ecs::EntityId CreatePokedexPokemonEntryDisplayTextbox
(
    ecs::World& world
);

void DestroyActiveTextbox
(
    ecs::World& world
);

// This is necessary since we sometimes want to delete 
// a textbox that is not active or is in the background
void DestroyGenericOrBareTextbox
(
    ecs::EntityId textboxEntityId,
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

void QueueDialogForChatbox
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
