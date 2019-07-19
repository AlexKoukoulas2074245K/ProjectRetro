//
//  TextboxComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 04/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef TextboxComponent_h
#define TextboxComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"

#include <vector>
#include <queue>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const int TEXTBOX_MIN_TILE_COLS = 3;
const int TEXTBOX_MIN_TILE_ROWS = 3;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class TextboxType
{
    GENERIC_TEXTBOX,
    BARE_TEXTBOX,
    CURSORED_TEXTBOX,
    CURSORED_BARE_TEXTBOX,
    CHATBOX
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class TextboxCharacterEntry
{
public:
    TextboxCharacterEntry()
        : mCharacter(0)
        , mEntityId(ecs::NULL_ENTITY_ID)
    {
    }
    
    TextboxCharacterEntry(const char character, const ecs::EntityId entityId)
        : mCharacter(character)
        , mEntityId(entityId)
    {
    }
    
    char mCharacter         = 0;
    ecs::EntityId mEntityId = ecs::NULL_ENTITY_ID;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

using TextboxContent        = std::vector<std::vector<TextboxCharacterEntry>>;
using TextboxQueuedTextLine = std::queue<char>;
using TextboxQueuedLines    = std::queue<TextboxQueuedTextLine>;
using TextboxQueuedDialog   = std::queue<TextboxQueuedLines>;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class TextboxComponent final: public ecs::IComponent
{
public:
    TextboxContent mTextContent;
    TextboxQueuedDialog mQueuedDialog;
    int mTextboxTileCols           = TEXTBOX_MIN_TILE_COLS;
    int mTextboxTileRows           = TEXTBOX_MIN_TILE_ROWS;
    TextboxType mTextboxType       = TextboxType::GENERIC_TEXTBOX;
    bool mShouldFreezeWhenComplete = false;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TextboxComponent_h */
