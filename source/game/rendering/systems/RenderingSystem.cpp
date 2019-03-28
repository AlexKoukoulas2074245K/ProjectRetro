//
//  RenderingSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "RenderingSystem.h"
#include "../opengl/Context.h"
#include "../../common_utils/MessageBox.h"
#include "../../common_utils/Logging.h"
#include "../components/WindowComponent.h"
#include "../components/RenderingContextComponent.h"

#include <SDL.h>   // Many SDL init related methods
#include <cstdlib> // exit

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

RenderingSystem::RenderingSystem(ecs::World& world)
    : ecs::BaseSystem(world)
{
    InitializeRenderingWindowAndContext();
}

void RenderingSystem::VUpdate(const float)
{

}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void RenderingSystem::InitializeRenderingWindowAndContext()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        ShowMessageBox(MessageBoxType::ERROR, "Error initializing SDL", "An error has occurred while trying to initialize SDL");
        exit(1);
    }

    // Set SDL GL attributes        
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Get Current Display Mode Resolution
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    const auto desiredWindowWidth  = static_cast<int>(displayMode.w * 0.66f);
    const auto desiredWindowHeight = static_cast<int>(displayMode.h * 0.66f);

    // Create SDL window
    auto windowComponent = std::make_unique<WindowComponent>();    
    windowComponent->mWindowHandle = SDL_CreateWindow("Hardcore2D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, desiredWindowWidth, desiredWindowHeight, SDL_WINDOW_OPENGL);
   
    if (windowComponent->mWindowHandle == nullptr)
    {
        ShowMessageBox(MessageBoxType::ERROR, "Error creating SDL window", "An error has occurred while trying to create an SDL_Window");
        exit(1);
    }

    // Make window non-resizable and display
    SDL_SetWindowResizable(windowComponent->mWindowHandle, SDL_FALSE);
    SDL_ShowWindow(windowComponent->mWindowHandle);

    // Create SDL GL context
    auto renderingContextComponent = std::make_unique<RenderingContextComponent>();
    renderingContextComponent->mGLContext = SDL_GL_CreateContext(windowComponent->mWindowHandle);
    if (renderingContextComponent->mGLContext == nullptr)
    {
        ShowMessageBox(MessageBoxType::ERROR, "Error creating SDL context", "An error has occurred while trying to create an SDL_Context");
        exit(1);
    }

    // Commit context 
    SDL_GL_MakeCurrent(windowComponent->mWindowHandle, renderingContextComponent->mGLContext);
    SDL_GL_SetSwapInterval(0);

#ifdef _WIN32
    // Initialize GLES2 function table
    glFuncTable.initialize();
#endif

    // Get actual render buffer width/height
    int renderableWidth  = 0;
    int renderableHeight = 0;
    SDL_GL_GetDrawableSize(windowComponent->mWindowHandle, &renderableWidth, &renderableHeight);

    windowComponent->mRenderableWidth  = static_cast<float>(renderableWidth);
    windowComponent->mRenderableHeight = static_cast<float>(renderableHeight);

    // Log GL driver info
    Log(LogType::INFO, "Vendor     : %s", GL_NO_CHECK(glGetString(GL_VENDOR)));
    Log(LogType::INFO, "Renderer   : %s", GL_NO_CHECK(glGetString(GL_RENDERER)));
    Log(LogType::INFO, "Version    : %s", GL_NO_CHECK(glGetString(GL_VERSION)));

    // Configure Blending
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    renderingContextComponent->mBlending = true;

    // Create Frame Buffer
    GL_CHECK(glGenFramebuffers(1, &renderingContextComponent->mFrameBufferId));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, renderingContextComponent->mFrameBufferId));

    // Create Frame Buffer Texture
    GL_CHECK(glGenTextures(1, &renderingContextComponent->mScreenRenderingTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, renderingContextComponent->mScreenRenderingTexture));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(windowComponent->mRenderableWidth), static_cast<GLsizei>(windowComponent->mRenderableHeight), 0, GL_RGB, GL_UNSIGNED_BYTE, 0));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderingContextComponent->mScreenRenderingTexture, 0));

    // Check frame buffer creation outcome
    if (GL_NO_CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
    {
        assert(false);
    }

    // Transfer ownership of singleton components to world
    mWorld.SetSingletonComponent<WindowComponent>(std::move(windowComponent));
    mWorld.SetSingletonComponent<RenderingContextComponent>(std::move(renderingContextComponent));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////