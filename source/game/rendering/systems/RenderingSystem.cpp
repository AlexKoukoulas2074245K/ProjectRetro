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
#include "../../common_utils/StringId.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../common_utils/FileUtils.h"
#include "../components/WindowComponent.h"
#include "../components/RenderingContextComponent.h"
#include "../components/ShaderStoreComponent.h"

#include <SDL.h>   // Many SDL init related methods
#include <cstdlib> // exit

GLfloat cube_vertices[] = {
    // front
    -1.0, -1.0,  1.0,
    1.0, -1.0,  1.0,
    1.0,  1.0,  1.0,
    -1.0,  1.0,  1.0,
    // back
    -1.0, -1.0, -1.0,
    1.0, -1.0, -1.0,
    1.0,  1.0, -1.0,
    -1.0,  1.0, -1.0
};

GLfloat cube_colors[] = {
    // front colors
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    1.0, 1.0, 1.0,
    // back colors
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    1.0, 1.0, 1.0
};

/* init_resources */
GLushort cube_elements[] = {
    // front
    0, 1, 2,
    2, 3, 0,
    // right
    1, 5, 6,
    6, 2, 1,
    // back
    7, 6, 5,
    5, 4, 7,
    // left
    4, 0, 3,
    3, 7, 4,
    // bottom
    4, 5, 1,
    1, 0, 4,
    // top
    3, 2, 6,
    6, 7, 3
};

struct attributes
{
    GLfloat coord3d[3];
    GLfloat color3d[3];
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

RenderingSystem::RenderingSystem(ecs::World& world)
    : ecs::BaseSystem(world)
{
    InitializeRenderingWindowAndContext();
    CompileAndLoadShaders();
}

void RenderingSystem::VUpdate(const float)
{
    const auto& renderingContextComponent = mWorld.GetSingletonComponent<RenderingContextComponent>();
    const auto& windowComponent           = mWorld.GetSingletonComponent<WindowComponent>();
    
    // Execute first pass rendering
    GL_CHECK(glClearColor(1.0f, 1.0f, 0.4f, 1.0f));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderingContextComponent.mIndexBufferObject));
    GL_CHECK(glDrawElements(GL_TRIANGLES, sizeof(cube_elements)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0));
    
    // Swap window buffers
    SDL_GL_SwapWindow(windowComponent.mWindowHandle);
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

    // Configure Depth
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    renderingContextComponent->mDepthTest = true;
    
    // Create VBO & IBO
    GL_CHECK(glGenBuffers(1, &renderingContextComponent->mVertexBufferObject));
    GL_CHECK(glGenBuffers(1, &renderingContextComponent->mIndexBufferObject));
    
    // Bind and Buffer VBO
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, renderingContextComponent->mVertexBufferObject));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW));    

    // Bind and Buffer IBO
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderingContextComponent->mIndexBufferObject));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW));  
 
    // Transfer ownership of singleton components to world
    mWorld.SetSingletonComponent<WindowComponent>(std::move(windowComponent));
    mWorld.SetSingletonComponent<RenderingContextComponent>(std::move(renderingContextComponent));

    // Now that the GL context has been initialized, the ResourceLoadingService
    // can be properly initialized (given that many of them call SDL services)
    ResourceLoadingService::GetInstance().InitializeResourceLoaders();
}

void RenderingSystem::CompileAndLoadShaders()
{
    const auto shaderNames    = GetAndFilterShaderNames();
    auto shaderStoreComponent = std::make_unique<ShaderStoreComponent>();
    
    for (const auto& shaderName: shaderNames)
    {
        auto shaderResourceId = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_SHADERS_ROOT + shaderName);
        auto& shaderResource = ResourceLoadingService::GetInstance().GetResource<ShaderResource>(shaderResourceId);
        
        shaderStoreComponent->shaders[StringId(shaderName)] = shaderResource;
        
        ResourceLoadingService::GetInstance().UnloadResource(shaderResourceId);
    }
    
    mWorld.SetSingletonComponent<ShaderStoreComponent>(std::move(shaderStoreComponent));
}

std::set<std::string> RenderingSystem::GetAndFilterShaderNames() const
{
    const auto vertexAndFragmentShaderFilenames = GetAllFilenamesInDirectory(ResourceLoadingService::RES_SHADERS_ROOT);

    std::set<std::string> shaderNames;
    for (const auto& shaderFilename : vertexAndFragmentShaderFilenames)
    {
        shaderNames.insert(GetFileNameWithoutExtension(shaderFilename));
    }
    return shaderNames;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
