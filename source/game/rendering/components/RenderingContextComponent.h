//
//  RenderingContextComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef RenderingContextComponent_h
#define RenderingContextComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

using SDL_GLContext = void*;
using GLuint        = unsigned int;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class RenderingContextComponent final : public ecs::IComponent
{
public:
    SDL_GLContext mGLContext       = nullptr;
    GLuint mFrameBufferId          = 0;
    GLuint mScreenRenderingTexture = 0;
    bool mBlending                 = true;
};

#endif /* RenderingContextComponent_h */