//
//  OverworldFlowControllerSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 08/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OverworldFlowControllerSystem.h"
#include "../components/OverworldFlowStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OverworldFlowControllerSystem::OverworldFlowControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    InitializeOverworldFlowState();
}

void OverworldFlowControllerSystem::VUpdateAssociatedComponents(const float dt) const
{
    auto& overworldFlowStateComponent = mWorld.GetSingletonComponent<OverworldFlowStateSingletonComponent>();
    if (overworldFlowStateComponent.mFlowHasJustFinished)
    {
        overworldFlowStateComponent.mFlowStateManager.SetActiveFlowState(nullptr);
    }
    else
    {
        overworldFlowStateComponent.mFlowStateManager.Update(dt);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void OverworldFlowControllerSystem::InitializeOverworldFlowState() const
{
    mWorld.SetSingletonComponent<OverworldFlowStateSingletonComponent>(std::make_unique<OverworldFlowStateSingletonComponent>());
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////