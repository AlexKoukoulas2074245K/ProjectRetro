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
#include "../components/ActiveLevelSingletonComponent.h"
#include "../components/OverworldFlowStateSingletonComponent.h"
#include "../utils/OverworldUtils.h"
#include "../../common/flowstates/ViridianCaterpieWeedleGuyOverworldFlowState.h"

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
    
    if (overworldFlowStateComponent.mFlowHookTriggered)
    {
        overworldFlowStateComponent.mFlowHookTriggered = false;
        DetermineWhichFlowToStart();
    }
    
    if (overworldFlowStateComponent.mFlowHasJustFinished)
    {
        overworldFlowStateComponent.mFlowHasJustFinished = false;
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

void OverworldFlowControllerSystem::DetermineWhichFlowToStart() const
{
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
        
    if (activeLevelComponent.mActiveLevelNameId == StringId("viridian"))
    { 
        StartOverworldFlowState<ViridianCaterpieWeedleGuyOverworldFlowState>(mWorld);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
