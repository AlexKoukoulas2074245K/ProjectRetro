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
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/FileUtils.h"
#include "../../common/utils/Logging.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/MessageBox.h"
#include "../../common/utils/StringUtils.h"
#include "../../resources/MeshResource.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../resources/TextureResource.h"
#include "../components/CameraComponent.h"
#include "../components/RenderableComponent.h"
#include "../components/RenderingContextComponent.h"
#include "../components/ShaderStoreComponent.h"
#include "../components/WindowComponent.h"
#include "../opengl/Context.h"

#include <cstdlib> // exit
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL.h> 

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

RenderingSystem::RenderingSystem(ecs::World& world)
    : ecs::BaseSystem(world)
{
    InitializeRenderingWindowAndContext();
    InitializeCamera();
    CompileAndLoadShaders();
    CalculateAndSetComponentUsageMask<RenderableComponent, TransformComponent>();
}

void RenderingSystem::VUpdate(const float)
{
    // Get common rendering singleton components
    const auto& windowComponent      = mWorld.GetSingletonComponent<WindowComponent>();
    const auto& cameraComponent      = mWorld.GetSingletonComponent<CameraComponent>();
    const auto& shaderStoreComponent = mWorld.GetSingletonComponent<ShaderStoreComponent>();
    
    // Calculate render-constant camera view matrix
    const auto view = glm::lookAtLH(cameraComponent.mPosition, cameraComponent.mFocusPosition, cameraComponent.mUpVector);
    
    // Execute first pass rendering
    GL_CHECK(glClearColor(1.0f, 1.0f, 0.4f, 1.0f));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    const auto& activeEntities = mWorld.GetActiveEntities();
    for (const auto& entityId: activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
            const auto& transformComponent  = mWorld.GetComponent<TransformComponent>(entityId);
            const auto& currentShader       = shaderStoreComponent.mShaders.at(renderableComponent.mShaderNameId);
            const auto& currentTexture      = ResourceLoadingService::GetInstance().GetResource<TextureResource>(renderableComponent.mTextureResourceId);
            const auto& currentMeshes       = renderableComponent.mMeshes;
            const auto& currentMesh         = ResourceLoadingService::GetInstance().GetResource<MeshResource>(currentMeshes[renderableComponent.mActiveMeshIndex]);
            
            // Use shader
            GL_CHECK(glUseProgram(currentShader.GetProgramId()));
            
            // Calculate world matrix for entity
            glm::mat4 world(1.0f);
            world = glm::translate(world, transformComponent.mPosition);
            world = glm::rotate(world, transformComponent.mRotation.x, X_AXIS);
            world = glm::rotate(world, transformComponent.mRotation.y, Y_AXIS);
            world = glm::rotate(world, transformComponent.mRotation.z, Z_AXIS);
            world = glm::scale(world, transformComponent.mScale);
            
            // Set rendering uniforms
            GL_CHECK(glUniformMatrix4fv(currentShader.GetUniformNamesToLocations().at(StringId("world")), 1, GL_FALSE, (GLfloat*)&world));
            GL_CHECK(glUniformMatrix4fv(currentShader.GetUniformNamesToLocations().at(StringId("view")), 1, GL_FALSE, (GLfloat*)&view));
            GL_CHECK(glUniformMatrix4fv(currentShader.GetUniformNamesToLocations().at(StringId("proj")), 1, GL_FALSE, (GLfloat*)&cameraComponent.mProjectionMatrix));
            GL_CHECK(glBindTexture(GL_TEXTURE_2D, currentTexture.GetGLTextureId()));
            
            // Perform indexed draw
            GL_CHECK(glBindVertexArray(currentMesh.GetVertexArrayObject()));
            GL_CHECK(glDrawElements(GL_TRIANGLES, currentMesh.GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
            
        }
    }
    
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
    
    // MS
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

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
    auto renderableWidth = 0;
    auto renderableHeight = 0;
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
    GL_CHECK(glDepthFunc(GL_LESS));
    renderingContextComponent->mDepthTest = true;
    
    // Transfer ownership of singleton components to world
    mWorld.SetSingletonComponent<WindowComponent>(std::move(windowComponent));
    mWorld.SetSingletonComponent<RenderingContextComponent>(std::move(renderingContextComponent));

    // Now that the GL context has been initialized, the ResourceLoadingService
    // can be properly initialized (given that many of them call SDL services)
    ResourceLoadingService::GetInstance().InitializeResourceLoaders();
}

void RenderingSystem::InitializeCamera()
{
    const auto& windowComponent = mWorld.GetSingletonComponent<WindowComponent>();
    auto cameraComponent = std::make_unique<CameraComponent>();
    
    cameraComponent->mPosition         = glm::vec3(0.0f, 5.0f, -5.0f);
    cameraComponent->mFocusPosition    = glm::vec3(0.0f, 0.0f, 0.0f);
    cameraComponent->mProjectionMatrix = glm::perspectiveFovLH(45.0f, windowComponent.mRenderableWidth, windowComponent.mRenderableHeight, 0.001f, 1000.0f);
    
    mWorld.SetSingletonComponent<CameraComponent>(std::move(cameraComponent));
}

void RenderingSystem::CompileAndLoadShaders()
{
    auto& renderingContextComponent = mWorld.GetSingletonComponent<RenderingContextComponent>();
    
    // Bind default VAO for correct shader compilation
    GL_CHECK(glGenVertexArrays(1, &renderingContextComponent.mDefaultVertexArrayObject));
    GL_CHECK(glBindVertexArray(renderingContextComponent.mDefaultVertexArrayObject));
    
    const auto shaderNames    = GetAndFilterShaderNames();
    auto shaderStoreComponent = std::make_unique<ShaderStoreComponent>();
    
    for (const auto& shaderName: shaderNames)
    {
        // By signaling to load either a .vs or a .fs, the ShaderLoader will load the pair automatically,
        // hence why the addition of the .vs here
        auto shaderResourceId = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_SHADERS_ROOT + shaderName + ".vs");
        auto& shaderResource = ResourceLoadingService::GetInstance().GetResource<ShaderResource>(shaderResourceId);
        
        // Save a copy of the shader to the ShaderStoreComponent
        shaderStoreComponent->mShaders[StringId(shaderName)] = shaderResource;
        
        // And unload the resource
        ResourceLoadingService::GetInstance().UnloadResource(shaderResourceId);
    }
    
    // Unbind any VAO currently bound
    GL_CHECK(glBindVertexArray(0));
    
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
