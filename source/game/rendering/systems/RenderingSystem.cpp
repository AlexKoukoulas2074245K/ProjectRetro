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
#include "../components/CameraSingletonComponent.h"
#include "../components/PreviousRenderingStateSingletonComponent.h"
#include "../components/RenderableComponent.h"
#include "../components/RenderingContextSingletonComponent.h"
#include "../components/ShaderStoreSingletonComponent.h"
#include "../components/WindowSingletonComponent.h"
#include "../opengl/Context.h"
#include "../utils/Colors.h"
#include "../utils/CameraUtils.h"
#include "../utils/RenderingUtils.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/FileUtils.h"
#include "../../common/utils/Logging.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/OSMessageBox.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/LevelResidentComponent.h"
#include "../../overworld/components/TransitionAnimationStateSingletonComponent.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../overworld/utils/TransitionAnimationUtils.h"
#include "../../resources/MeshResource.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../resources/TextureResource.h"
#include "../../sound/SoundService.h"

#include <algorithm> // sort
#include <cstdlib>   // exit
#include <SDL.h> 
#include <vector>
#include <iterator>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const StringId RenderingSystem::WORLD_MARIX_UNIFORM_NAME                     = StringId("world");
const StringId RenderingSystem::VIEW_MARIX_UNIFORM_NAME                      = StringId("view");
const StringId RenderingSystem::PROJECTION_MARIX_UNIFORM_NAME                = StringId("proj");
const StringId RenderingSystem::TRANSITION_ANIMATION_STEP_UNIFORM_NAME       = StringId("transition_progression_step");
const StringId RenderingSystem::DARK_FLIP_STEP_UNIFORM_NAME                  = StringId("dark_flip_progression_step");
const StringId RenderingSystem::WHITE_FLIP_STEP_UNIFORM_NAME                 = StringId("white_flip_progression_step");
const StringId RenderingSystem::BLACK_AND_WHITE_MODE_UNIFORM_NAME            = StringId("black_and_white_mode");
const StringId RenderingSystem::CURRENT_LEVEL_COLOR_UNIFORM_NAME             = StringId("current_level_color");
const StringId RenderingSystem::PRIMARY_LIGHT_COLOR_UNIFORM_NAME             = StringId("primary_light_color");
const StringId RenderingSystem::PRIMARY_DARK_COLOR_UNIFORM_NAME              = StringId("primary_dark_color");
const StringId RenderingSystem::OVERRIDDEN_LIGHT_COLOR_UNIFORM_NAME          = StringId("overridden_light_color");
const StringId RenderingSystem::OVERRIDDEN_DARK_COLOR_UNIFORM_NAME           = StringId("overridden_dark_color");
const StringId RenderingSystem::SHOULD_OVERRIDE_PRIMARY_COLORS_UNIFORMN_NAME = StringId("should_override_primary_colors");
const StringId RenderingSystem::SPRITE_SHADER_NAME                           = StringId("basic");

const std::unordered_set<StringId, StringIdHasher> RenderingSystem::GUI_SHADERS =
{
    StringId("gui"),
    StringId("dark_flip_hud_special_case"),
    StringId("gui_unaffected_by_white_flip"),
    StringId("transition_flip_hud_special_case"),
    StringId("intro_sequence")
};

const float RenderingSystem::TARGET_ASPECT_RATIO = 1.5993266f;

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

void RenderingSystem::VUpdateAssociatedComponents(const float) const
{
    // Get common rendering singleton components
    const auto& windowComponent               = mWorld.GetSingletonComponent<WindowSingletonComponent>();
    const auto& shaderStoreComponent          = mWorld.GetSingletonComponent<ShaderStoreSingletonComponent>();
    const auto& activeLevelSingletonComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& transitionAnimationComponent  = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    auto& cameraComponent                     = mWorld.GetSingletonComponent<CameraSingletonComponent>();
    auto& renderingContextComponent           = mWorld.GetSingletonComponent<RenderingContextSingletonComponent>();
    auto& previousRenderingStateComponent     = mWorld.GetSingletonComponent<PreviousRenderingStateSingletonComponent>();
    
    // Calculate render-constant camera view matrix
    cameraComponent.mViewMatrix = glm::lookAtLH(cameraComponent.mPosition, cameraComponent.mFocusPosition, cameraComponent.mUpVector);
    
    // Calculate the camera frustum for this frame
    cameraComponent.mFrustum = CalculateCameraFrustum(cameraComponent.mViewMatrix, cameraComponent.mProjectionMatrix);
        
    // Debug statistics
    renderingContextComponent.mFrustumCulledEntities = 0U;
    renderingContextComponent.mRenderedEntities      = 0U;
    
    // Collect all entities that need to be processed
    const auto& activeEntities = mWorld.GetActiveEntities();
    std::vector<ecs::EntityId> guiEntities;
    std::vector<ecs::EntityId> topRenderedEntities;
    std::vector<ecs::EntityId> semiTransparentTexturedEntities;
    
    // Set background color
    const auto& currentLevel    = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelSingletonComponent.mActiveLevelNameId, mWorld));
    const auto& backgroundColor = GetBackgroundColorBasedOnTransitionStep(currentLevel.mLevelColor, currentLevel.mLevelName, transitionAnimationComponent);
    GL_CHECK(glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w));
    
    // Clear buffers
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Enable depth
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    
    // First check and render any underground entities (staircases down, etc)
    for (const auto& entityId : activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
            if (renderableComponent.mRenderableLayer == RenderableLayer::UNDERGROUND)
            {
                const auto& transformComponent = mWorld.GetComponent<TransformComponent>(entityId);
                const auto& activeMeshes       = renderableComponent.mAnimationsToMeshes.at(renderableComponent.mActiveAnimationNameId);
                const auto& currentMesh        = ResourceLoadingService::GetInstance().GetResource<MeshResource>(activeMeshes[renderableComponent.mActiveMeshIndex]);

                // Frustum culling
                if (!IsMeshInsideCameraFrustum
                (
                    transformComponent.mPosition,
                    transformComponent.mScale,
                    currentMesh.GetDimensions(),
                    cameraComponent.mFrustum
                ))
                {
                    renderingContextComponent.mFrustumCulledEntities++;
                    continue;
                }

                RenderEntityInternal
                (
                    entityId,
                    renderableComponent,
                    currentLevel.mLevelColor,
                    cameraComponent,
                    shaderStoreComponent,
                    windowComponent,
                    transitionAnimationComponent,
                    renderingContextComponent,
                    previousRenderingStateComponent
                );
            }            
        }
    }

    // Then, render the level ground floor
    RenderEntityInternal
    (
        currentLevel.mGroundLayerEntity,
        mWorld.GetComponent<RenderableComponent>(currentLevel.mGroundLayerEntity),
        currentLevel.mLevelColor,
        cameraComponent,
        shaderStoreComponent,
        windowComponent,
        transitionAnimationComponent,
        renderingContextComponent,
        previousRenderingStateComponent
    );

    // Then check and render any wild grass entities 
    for (const auto& entityId : activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
            if (renderableComponent.mRenderableLayer == RenderableLayer::WILD_GRASS)
            {
                const auto& transformComponent = mWorld.GetComponent<TransformComponent>(entityId);
                const auto& activeMeshes = renderableComponent.mAnimationsToMeshes.at(renderableComponent.mActiveAnimationNameId);
                const auto& currentMesh = ResourceLoadingService::GetInstance().GetResource<MeshResource>(activeMeshes[renderableComponent.mActiveMeshIndex]);

                // Frustum culling
                if (!IsMeshInsideCameraFrustum
                (
                    transformComponent.mPosition,
                    transformComponent.mScale,
                    currentMesh.GetDimensions(),
                    cameraComponent.mFrustum
                ))
                {
                    renderingContextComponent.mFrustumCulledEntities++;
                    continue;
                }

                RenderEntityInternal
                (
                    entityId,
                    renderableComponent,
                    currentLevel.mLevelColor,
                    cameraComponent,
                    shaderStoreComponent,
                    windowComponent,
                    transitionAnimationComponent,
                    renderingContextComponent,
                    previousRenderingStateComponent
                );
            }
        }
    }
    
    for (const auto& entityId: activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
            const auto& transformComponent  = mWorld.GetComponent<TransformComponent>(entityId);
            const auto& currentTexture      = ResourceLoadingService::GetInstance().GetResource<TextureResource>(renderableComponent.mTextureResourceId);
            const auto& activeMeshes        = renderableComponent.mAnimationsToMeshes.at(renderableComponent.mActiveAnimationNameId);            
            const auto& currentMesh         = ResourceLoadingService::GetInstance().GetResource<MeshResource>(activeMeshes[renderableComponent.mActiveMeshIndex]);

            // Ignore underground layered entities now
            if (renderableComponent.mRenderableLayer == RenderableLayer::UNDERGROUND)
            {                
                continue;
            }

            if (entityId == currentLevel.mGroundLayerEntity)
            {
                continue;
            }
                
            // Separate GUI entities to render them last
            if (GUI_SHADERS.count(renderableComponent.mShaderNameId) != 0)
            {
                guiEntities.push_back(entityId);
                continue;
            }
            
            // Check level residency and reject if applicable
            if (mWorld.HasComponent<LevelResidentComponent>(entityId) &&
                mWorld.GetComponent<LevelResidentComponent>(entityId).mLevelNameId != activeLevelSingletonComponent.mActiveLevelNameId)
            {
                continue;
            }            

            // Frustum culling
            if (!IsMeshInsideCameraFrustum
            (
                    transformComponent.mPosition,
                    transformComponent.mScale,
                    currentMesh.GetDimensions(),
                    cameraComponent.mFrustum                
            ))
            {
                renderingContextComponent.mFrustumCulledEntities++;
                continue;
            }

            // If entity is solid-textured (has no semi transparent pixels) or it is
            // part of the level geometry textures then it is rendered in this pass here. 
            // Otherwise the entity is added to a vector of semi-transparent entities that 
            // need to be sorted prior to rendering.
            if (currentTexture.HasTransparentPixels())
            {
                semiTransparentTexturedEntities.push_back(entityId);
            }
            else
            {
                RenderEntityInternal
                (
                    entityId, 
                    renderableComponent, 
                    currentLevel.mLevelColor,
                    cameraComponent, 
                    shaderStoreComponent, 
                    windowComponent,
                    transitionAnimationComponent,
                    renderingContextComponent,
                    previousRenderingStateComponent
                );
            }            
        }
    }
        
    // Sort entities based on their depth order to correct transparency
    std::sort(semiTransparentTexturedEntities.begin(), semiTransparentTexturedEntities.end(), [this](const ecs::EntityId& lhs, const ecs::EntityId& rhs)
    {
        const auto& lhsTransformComponent = mWorld.GetComponent<TransformComponent>(lhs);
        const auto& rhsTransformComponent = mWorld.GetComponent<TransformComponent>(rhs);

        return lhsTransformComponent.mPosition.z > rhsTransformComponent.mPosition.z;
    });


    // Execute semi-tranasparent render pass
    for (const auto& entityId : semiTransparentTexturedEntities)
    {
        const auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
        RenderEntityInternal
        (
            entityId, 
            renderableComponent, 
            currentLevel.mLevelColor,
            cameraComponent, 
            shaderStoreComponent, 
            windowComponent,
            transitionAnimationComponent,
            renderingContextComponent,
            previousRenderingStateComponent
        );
    }            

    // Sort GUI elements based on z
    std::sort(guiEntities.begin(), guiEntities.end(), [this](const ecs::EntityId& lhs, const ecs::EntityId& rhs)
    {
        const auto& lhsTransformComponent = mWorld.GetComponent<TransformComponent>(lhs);
        const auto& rhsTransformComponent = mWorld.GetComponent<TransformComponent>(rhs);
                  
        return lhsTransformComponent.mPosition.z > rhsTransformComponent.mPosition.z;
    });
    
    // Execute GUI render pass
    GL_CHECK(glDisable(GL_DEPTH_TEST));
    
    for (const auto& entityId : guiEntities)
    {
        const auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(entityId);
        RenderEntityInternal
        (
            entityId,
            renderableComponent,
            currentLevel.mLevelColor,
            cameraComponent,
            shaderStoreComponent,
            windowComponent,
            transitionAnimationComponent,
            renderingContextComponent,
            previousRenderingStateComponent
        );
    }

    // Swap window buffers
    SDL_GL_SwapWindow(windowComponent.mWindowHandle);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void RenderingSystem::RenderEntityInternal
(
    const ecs::EntityId entityId,
    const RenderableComponent& renderableComponent,
    const glm::vec4& currentLevelColor,
    const CameraSingletonComponent& cameraComponent, 
    const ShaderStoreSingletonComponent& shaderStoreComponent,
    const WindowSingletonComponent& windowComponent,
    const TransitionAnimationStateSingletonComponent& transitionAnimationComponent,
    RenderingContextSingletonComponent& renderingContextComponent,
    PreviousRenderingStateSingletonComponent& previousRenderingStateComponent
) const
{
    if (!renderableComponent.mVisibility)
    {
        return;
    }

    // Update Shader is necessary
    const ShaderResource* currentShader = nullptr;
    if (renderableComponent.mShaderNameId != previousRenderingStateComponent.previousShaderNameId)
    {
        currentShader = &shaderStoreComponent.mShaders.at(renderableComponent.mShaderNameId);
        GL_CHECK(glUseProgram(currentShader->GetProgramId()));

        previousRenderingStateComponent.previousShaderNameId = renderableComponent.mShaderNameId;
        previousRenderingStateComponent.previousShader       = currentShader;        
    }
    else
    {
        currentShader = previousRenderingStateComponent.previousShader;
    }

    // Get currently active mesh for this entity
    const auto& transformComponent = mWorld.GetComponent<TransformComponent>(entityId);    
    const auto& activeMeshes       = renderableComponent.mAnimationsToMeshes.at(renderableComponent.mActiveAnimationNameId);

    // Update current mesh if necessary
    const MeshResource* currentMesh = nullptr;
    if (activeMeshes[renderableComponent.mActiveMeshIndex] != previousRenderingStateComponent.previousMeshResourceId)
    {
        currentMesh = &ResourceLoadingService::GetInstance().GetResource<MeshResource>(activeMeshes[renderableComponent.mActiveMeshIndex]);
        GL_CHECK(glBindVertexArray(currentMesh->GetVertexArrayObject()));

        previousRenderingStateComponent.previousMesh           = currentMesh;
        previousRenderingStateComponent.previousMeshResourceId = activeMeshes[renderableComponent.mActiveMeshIndex];
    }
    else
    {
        currentMesh = previousRenderingStateComponent.previousMesh;
    }
        
    // Calculate world matrix for entity
    glm::mat4 world(1.0f);
        
    // Correct display of hud and billboard entities    
    glm::vec3 position = transformComponent.mPosition;
    glm::vec3 scale    = transformComponent.mScale;
    glm::vec3 rotation = transformComponent.mRotation;

    if (!renderableComponent.mAffectedByPerspective)
    {        
        scale.x /= windowComponent.mAspectRatio;        
    }  

    world = glm::translate(world, position + cameraComponent.mGlobalScreenOffset);
    world = glm::rotate(world, rotation.x, math::X_AXIS);
    world = glm::rotate(world, rotation.y, math::Y_AXIS);
    world = glm::rotate(world, rotation.z, math::Z_AXIS);
    world = glm::scale(world, scale);
        
    // Update texture if necessary
    const TextureResource* currentTexture = nullptr;
    if (renderableComponent.mTextureResourceId != previousRenderingStateComponent.previousTextureResourceId)
    {
        currentTexture = &ResourceLoadingService::GetInstance().GetResource<TextureResource>(renderableComponent.mTextureResourceId);
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, currentTexture->GetGLTextureId()));

        previousRenderingStateComponent.previousTexture = currentTexture;
        previousRenderingStateComponent.previousTextureResourceId = renderableComponent.mTextureResourceId;        
    }
    else
    {
        currentTexture = previousRenderingStateComponent.previousTexture;
    }      

    // Set uniforms
    GL_CHECK(glUniform1i(currentShader->GetUniformNamesToLocations().at(SHOULD_OVERRIDE_PRIMARY_COLORS_UNIFORMN_NAME), renderableComponent.mShouldOverrideDarkAndLightColor ? 1 : 0));
    GL_CHECK(glUniform1i(currentShader->GetUniformNamesToLocations().at(TRANSITION_ANIMATION_STEP_UNIFORM_NAME), transitionAnimationComponent.mAnimationProgressionStep));
    GL_CHECK(glUniform1i(currentShader->GetUniformNamesToLocations().at(DARK_FLIP_STEP_UNIFORM_NAME), transitionAnimationComponent.mDarkFlipProgressionStep));
    GL_CHECK(glUniform1i(currentShader->GetUniformNamesToLocations().at(WHITE_FLIP_STEP_UNIFORM_NAME), transitionAnimationComponent.mWhiteFlipProgressionStep));
    GL_CHECK(glUniform1i(currentShader->GetUniformNamesToLocations().at(BLACK_AND_WHITE_MODE_UNIFORM_NAME), transitionAnimationComponent.mBlackAndWhiteModeEnabled ? 1 : 0));
    GL_CHECK(glUniform4f(currentShader->GetUniformNamesToLocations().at(CURRENT_LEVEL_COLOR_UNIFORM_NAME), currentLevelColor.x, currentLevelColor.y, currentLevelColor.z, currentLevelColor.w));    
    GL_CHECK(glUniformMatrix4fv(currentShader->GetUniformNamesToLocations().at(WORLD_MARIX_UNIFORM_NAME), 1, GL_FALSE, (GLfloat*)&world));
    GL_CHECK(glUniformMatrix4fv(currentShader->GetUniformNamesToLocations().at(VIEW_MARIX_UNIFORM_NAME), 1, GL_FALSE, (GLfloat*)&cameraComponent.mViewMatrix));
    GL_CHECK(glUniformMatrix4fv(currentShader->GetUniformNamesToLocations().at(PROJECTION_MARIX_UNIFORM_NAME), 1, GL_FALSE, (GLfloat*)&cameraComponent.mProjectionMatrix));

    glm::vec4 primaryLightColor, primaryDarkColor;
    GetPrimaryLightAndPrimaryDarkColorsFromSet(currentTexture->GetColorSet(), primaryLightColor, primaryDarkColor);

    GL_CHECK(glUniform4f(currentShader->GetUniformNamesToLocations().at(PRIMARY_LIGHT_COLOR_UNIFORM_NAME), primaryLightColor.x, primaryLightColor.y, primaryLightColor.z, primaryLightColor.w));
    GL_CHECK(glUniform4f(currentShader->GetUniformNamesToLocations().at(PRIMARY_DARK_COLOR_UNIFORM_NAME), primaryDarkColor.x, primaryDarkColor.y, primaryDarkColor.z, primaryDarkColor.w));
    GL_CHECK(glUniform4f(currentShader->GetUniformNamesToLocations().at(OVERRIDDEN_LIGHT_COLOR_UNIFORM_NAME), renderableComponent.mOverriddenLightColor.x, renderableComponent.mOverriddenLightColor.y, renderableComponent.mOverriddenLightColor.z, renderableComponent.mOverriddenLightColor.w));
    GL_CHECK(glUniform4f(currentShader->GetUniformNamesToLocations().at(OVERRIDDEN_DARK_COLOR_UNIFORM_NAME), renderableComponent.mOverriddenDarkColor.x, renderableComponent.mOverriddenDarkColor.y, renderableComponent.mOverriddenDarkColor.z, renderableComponent.mOverriddenDarkColor.w));

    renderingContextComponent.mRenderedEntities++;
    
    // Perform draw call
    GL_CHECK(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
}

void RenderingSystem::InitializeRenderingWindowAndContext() const
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
    const auto desiredWindowHeight = static_cast<int>(desiredWindowWidth/TARGET_ASPECT_RATIO);

    // Create SDL window
    auto windowComponent           = std::make_unique<WindowSingletonComponent>();   
    windowComponent->mWindowTitle  = "ProjectRetro";
    windowComponent->mWindowHandle = SDL_CreateWindow(windowComponent->mWindowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, desiredWindowWidth, desiredWindowHeight, SDL_WINDOW_OPENGL);
   
    if (windowComponent->mWindowHandle == nullptr)
    {
        ShowMessageBox(MessageBoxType::ERROR, "Error creating SDL window", "An error has occurred while trying to create an SDL_Window");
        exit(1);
    }

    // Make window non-resizable and display
    SDL_SetWindowResizable(windowComponent->mWindowHandle, SDL_FALSE);
    SDL_ShowWindow(windowComponent->mWindowHandle);

    // Create SDL GL context
    auto renderingContextComponent = std::make_unique<RenderingContextSingletonComponent>();
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
    auto renderableWidth  = 0;
    auto renderableHeight = 0;
    SDL_GL_GetDrawableSize(windowComponent->mWindowHandle, &renderableWidth, &renderableHeight);

    windowComponent->mRenderableWidth  = static_cast<float>(renderableWidth);
    windowComponent->mRenderableHeight = static_cast<float>(renderableHeight);
    windowComponent->mAspectRatio      = windowComponent->mRenderableWidth/windowComponent->mRenderableHeight;
    
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
    mWorld.SetSingletonComponent<WindowSingletonComponent>(std::move(windowComponent));
    mWorld.SetSingletonComponent<RenderingContextSingletonComponent>(std::move(renderingContextComponent));
    mWorld.SetSingletonComponent<PreviousRenderingStateSingletonComponent>(std::make_unique<PreviousRenderingStateSingletonComponent>());

    // Now that the GL context has been initialized, the ResourceLoadingService
    // can be properly initialized (given that many of them call SDL services)
    ResourceLoadingService::GetInstance().InitializeResourceLoaders();
    SoundService::GetInstance().InitializeSdlMixer();
}

void RenderingSystem::InitializeCamera() const
{
    const auto& windowComponent = mWorld.GetSingletonComponent<WindowSingletonComponent>();
    auto cameraComponent = std::make_unique<CameraSingletonComponent>();
    cameraComponent->mProjectionMatrix = glm::perspectiveFovLH
    (
        cameraComponent->mFieldOfView,
        windowComponent.mRenderableWidth,
        windowComponent.mRenderableHeight,
        cameraComponent->mZNear,
        cameraComponent->mZFar
    );
    
    mWorld.SetSingletonComponent<CameraSingletonComponent>(std::move(cameraComponent));
}

void RenderingSystem::CompileAndLoadShaders() const
{
    auto& renderingContextComponent = mWorld.GetSingletonComponent<RenderingContextSingletonComponent>();
    
    // Bind default VAO for correct shader compilation
    GL_CHECK(glGenVertexArrays(1, &renderingContextComponent.mDefaultVertexArrayObject));
    GL_CHECK(glBindVertexArray(renderingContextComponent.mDefaultVertexArrayObject));
    
    const auto shaderNames    = GetAndFilterShaderNames();
    auto shaderStoreComponent = std::make_unique<ShaderStoreSingletonComponent>();
    
    for (const auto& shaderName: shaderNames)
    {
        // By signaling to load either a .vs or a .fs, the ShaderLoader will load the pair automatically,
        // hence why the addition of the .vs here
        auto shaderResourceId = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_SHADERS_ROOT + shaderName + ".vs");
        auto& shaderResource  = ResourceLoadingService::GetInstance().GetResource<ShaderResource>(shaderResourceId);
        
        // Save a copy of the shader to the ShaderStoreComponent
        shaderStoreComponent->mShaders[StringId(shaderName)] = shaderResource;
        
        // And unload the resource
        ResourceLoadingService::GetInstance().UnloadResource(shaderResourceId);
    }
    
    // Unbind any VAO currently bound
    GL_CHECK(glBindVertexArray(0));
    
    mWorld.SetSingletonComponent<ShaderStoreSingletonComponent>(std::move(shaderStoreComponent));
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
