//
//  FlowStateManager.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 04/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "FlowStateManager.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

FlowStateManager::FlowStateManager()
    : mActiveFlowState(nullptr)
{

}

FlowStateManager::FlowStateManager(std::unique_ptr<BaseFlowState> initialState)
    : mActiveFlowState(std::move(initialState))
{

}

bool FlowStateManager::HasActiveFlowState() const
{
    return mActiveFlowState != nullptr;
}

void FlowStateManager::SetActiveFlowState(std::unique_ptr<BaseFlowState> flowState)
{
    mActiveFlowState = std::move(flowState);
}

void FlowStateManager::Update(const float dt)
{
    if (HasActiveFlowState())
    {
        mActiveFlowState->VUpdate(dt);

        // While instead of if to handle completion on construction
        while (mActiveFlowState->mNextFlowState != nullptr)
        {
            SetActiveFlowState(std::move(mActiveFlowState->mNextFlowState));
        }
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
