//
//  RenderingSystem.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef RenderingSystem_h
#define RenderingSystem_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../common/utils/StringUtils.h"
#include "../../common/utils/MathUtils.h"
#include "../../ECS.h"

#include <set>
#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class CameraSingletonComponent;
class PreviousRenderingStateSingletonComponent;
class RenderableComponent;
class RenderingContextSingletonComponent;
class ShaderStoreSingletonComponent;
class TransitionAnimationStateSingletonComponent;
class WindowSingletonComponent;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class RenderingSystem final: public ecs::BaseSystem
{
public:
    RenderingSystem(ecs::World& world);
    
    void VUpdateAssociatedComponents(const float dt) const override;

private:
    void RenderEntityInternal
    (
        const ecs::EntityId entityId,
        const RenderableComponent& entityRenderableComponent,
        const glm::vec4& currentLevelColor,
        const CameraSingletonComponent& globalCameraComponent,
        const ShaderStoreSingletonComponent& globalShaderStoreComponent,
        const WindowSingletonComponent& globalWindowComponent,
        const TransitionAnimationStateSingletonComponent& transitionAnimationComponent,
        RenderingContextSingletonComponent& renderingContextComponent,
        PreviousRenderingStateSingletonComponent& globalPreviousRenderingStateComponent
    ) const;
        
    void InitializeRenderingWindowAndContext() const;
    void InitializeCamera() const;
    void CompileAndLoadShaders() const;

    std::set<std::string> GetAndFilterShaderNames() const;

private:
    static const StringId WORLD_MARIX_UNIFORM_NAME;
    static const StringId VIEW_MARIX_UNIFORM_NAME;
    static const StringId PROJECTION_MARIX_UNIFORM_NAME;
    static const StringId TRANSITION_ANIMATION_STEP_UNIFORM_NAME;
    static const StringId CURRENT_LEVEL_COLOR_UNIFORM_NAME;      
    static const StringId GUI_SHADER_NAME;

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* RenderingSystem_h */
