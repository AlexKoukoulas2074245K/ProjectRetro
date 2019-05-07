//
//  MeshUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 05/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef MeshUtils_h
#define MeshUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../common/utils/StringUtils.h"
#include "../resources/ResourceLoadingService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class RenderableComponent;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ResourceId LoadMeshFromAtlasTexCoords
(
    const int meshAtlasCol,
    const int meshAtlasRow,
    const int atlasCols,
    const int atlasRows,
    const bool horFlipped,
    const std::string& modelName
);

ResourceId LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations
(
    const int meshAtlasCol,
    const int meshAtlasRow,
    const int atlasCols,
    const int atlasRows,
    const bool horFlipped,
    const std::string& modelName,
    const StringId animationNameId,
    RenderableComponent& renderableComponent
);


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* MeshUtils_h */
