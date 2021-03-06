//
//  PreviousRenderingStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PreviousRenderingStateSingletonComponent_h
#define PreviousRenderingStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/utils/StringUtils.h"
#include "../../resources/ResourceLoadingService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class ShaderResource;
class TextureResource;
class MeshResource;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PreviousRenderingStateSingletonComponent final : public ecs::IComponent
{
public:
    StringId previousShaderNameId        = StringId();
    ResourceId previousTextureResourceId = ResourceId();
    ResourceId previousMeshResourceId    = ResourceId();
    const ShaderResource* previousShader       = nullptr;
    const TextureResource* previousTexture     = nullptr;
    const MeshResource* previousMesh           = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PreviousRenderingStateSingletonComponent_h */