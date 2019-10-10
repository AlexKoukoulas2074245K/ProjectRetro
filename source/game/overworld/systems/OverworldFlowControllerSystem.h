//
//  OverworldFlowControllerSystem.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 08/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef OverworldFlowControllerSystem_h
#define OverworldFlowControllerSystem_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/utils/StringUtils.h"

#include <functional>
#include <memory>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class BaseFlowState;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class OverworldFlowControllerSystem final: public ecs::BaseSystem
{
public:
    OverworldFlowControllerSystem(ecs::World&);
    
    void VUpdateAssociatedComponents(const float dt) const override;
 
private:
    void InitializeOverworldFlowState() const;
	void RegisterNamedFlowStateFactories();
	void UpdateExposedNamedFlowStatesFile() const;
    void DetermineWhichFlowToStart() const;

	using FlowStateFactoryFunction = std::function<std::unique_ptr<BaseFlowState>()>;
	std::unordered_map<StringId, FlowStateFactoryFunction, StringIdHasher> mNamedFlowStatesFactory;

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* OverworldFlowControllerSystem_h */
