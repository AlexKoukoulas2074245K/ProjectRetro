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
#include "../GameConstants.h"
#include "../../ECS.h"
#include "../../overworld/components/PCStateSingletonComponent.h"

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class TextboxCharacterEntry;
struct Pokemon;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

extern const glm::vec3 CHATBOX_POSITION; 
extern const glm::vec3 ENCOUNTER_FIGHT_MENU_TEXTBOX_POSITION;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class SpecialCharacter
{
    CHARACTER_STAND_UP, CHARACTER_STAND_NORMAL, POKEDEX_CAUGHT_BALL
};

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

ecs::EntityId CreatePokeMartItemQuantityTextbox
(    
    ecs::World& world,
    const int itemQuantity,
    const int itemPrice    
);

ecs::EntityId CreatePCMainOptionsTextbox
(
    ecs::World& world,
    const bool billInteractionCompleted
);

ecs::EntityId CreatePCPokemonSystemOptionsTextbox
(
    ecs::World& world,    
    const int cursorRow = 0
);

ecs::EntityId CreatePCPokemonSystemPokemonListTextbox
(
    ecs::World& world,
    const size_t count,
    const int previousCursorRow = 0,
    const int itemOffset = 0
);

ecs::EntityId CreatePCPokemonSelectedOptionsTextbox
(
    ecs::World& world,
    const PokemonSystemOperationType operationType,
    const int cursorRow = 0
);

ecs::EntityId CreatePokedexMainViewInvisibleListTextbox
(    
    ecs::World& world,
    const int previousCursorRow = 0,
    const int itemOffset = 0 
);

ecs::EntityId CreatePokedexSelectionViewInvisibleTextbox
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
    const int intActionTypeSelected,
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

ecs::EntityId CreateNameSelectionCharactersEnclosingTextbox
(
    ecs::World& world
);

ecs::EntityId CreateNameSelectionCharactersInvisibleTextbox
(
    const bool uppercaseMode,
    ecs::World& world,
    const int previousCursorCol = 0,
    const int previousCursorRow = 0
);

ecs::EntityId CreateNameSelectionTitleInvisibleTextbox
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

void WriteSpecialCharacterAtTextboxCoords
(
    const ecs::EntityId textboxEntityId,
    const SpecialCharacter specialCharacter,
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

void MoveTextboxCursorToDirection
(
    const ecs::EntityId textboxEntityId,
    const Direction direction,
    ecs::World& world
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TextboxUtils_h */
