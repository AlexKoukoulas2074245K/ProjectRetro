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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const int TEXTBOX_MIN_TILE_COLS = 3;
const int TEXTBOX_MIN_TILE_ROWS = 3;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

// Basic chat is 20x6 half tiles (8x8)
// In most dialogs writable letters can be placed
// from (1,2) to (18,2) and
// from (1,4) to (18,4)
// in general. letters can be placed anywhere inside
// the bounds (1,1) to (width - 1, height - 1).
// The smallest viable chat is 3,3
// I'm BROCK!{s}I'm PEWTER's GYM LEADER{p}I believe in rock hard defense and determination!{p}
// Thats why my POKEMON are all the rock-type!{p}
// Do you still want to challenge me? Fine then! show me your best!

// GARY: Gramps!{s}I'm fed up with waiting
class TextboxComponent final: public ecs::IComponent
{
public:
    std::vector<std::vector<char>> mTextContent;
    
    int mTextboxTileCols = TEXTBOX_MIN_TILE_COLS;
    int mTextboxTileRows = TEXTBOX_MIN_TILE_ROWS;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TextboxComponent_h */
