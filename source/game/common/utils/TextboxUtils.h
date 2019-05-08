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

ecs::EntityId CreateTextboxWithDimensions
(
    const int textboxTileCols,
    const int textboxTileRows,
    const float textboxOriginX,
    const float textboxOriginY,
    ecs::World& world
);

void WriteTextAtTextboxCoords
(
    const std::string& text,
    const size_t textboxCol,
    const size_t textboxRow,
    TextboxContent& textboxContent
);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TextboxUtils_h */
