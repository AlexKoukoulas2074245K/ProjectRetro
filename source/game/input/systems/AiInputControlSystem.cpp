//
//  AiInputControlSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 22/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "AiInputControlSystem.h"
#include "../components/AiInputControlStateSingletonComponent.h"
#include "../components/InputStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float AiInputControlSystem::OAK_PIKACHU_CAPTURE_INPUT_DELAY = 0.5f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

AiInputControlSystem::AiInputControlSystem(ecs::World& world)
    : BaseSystem(world)
{
    auto aiInputControlStateComponent = std::make_unique<AiInputControlStateSingletonComponent>();
    mWorld.SetSingletonComponent<AiInputControlStateSingletonComponent>(std::move(aiInputControlStateComponent));
}

void AiInputControlSystem::VUpdateAssociatedComponents(const float dt) const
{       
    const auto& aiInputControlStateComponent = mWorld.GetSingletonComponent<AiInputControlStateSingletonComponent>();    

    if (aiInputControlStateComponent.mAIInputControllerType == AiInputControllerType::AI_OAK_PIKACHU_CAPTURE)
    {
        UpdateAiOakPikachuCaptureController(dt);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void AiInputControlSystem::UpdateAiOakPikachuCaptureController(const float dt) const
{
    ResetInputState();

    auto& inputStateComponent          = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& aiInputControlStateComponent = mWorld.GetSingletonComponent<AiInputControlStateSingletonComponent>();

    switch (aiInputControlStateComponent.mAiControllerInputProgressionStep)
    {
        // Timer Setup
        case 0: 
        {
            aiInputControlStateComponent.mAiControllerInputDelayTimer = std::make_unique<Timer>(OAK_PIKACHU_CAPTURE_INPUT_DELAY);
            aiInputControlStateComponent.mAiControllerInputProgressionStep++;
        }

        // Down arrow key to item Menu
        case 1:
        {
            aiInputControlStateComponent.mAiControllerInputDelayTimer->Update(dt);
            if (aiInputControlStateComponent.mAiControllerInputDelayTimer->HasTicked())
            {
                aiInputControlStateComponent.mAiControllerInputDelayTimer->Reset();
                inputStateComponent.mCurrentInputState[VirtualActionType::DOWN_ARROW] = VirtualActionInputState::TAPPED;
                aiInputControlStateComponent.mAiControllerInputProgressionStep++;
                
            }
        }

        // A-button to open item menu        
        case 2:
        {
            aiInputControlStateComponent.mAiControllerInputDelayTimer->Update(dt);
            if (aiInputControlStateComponent.mAiControllerInputDelayTimer->HasTicked())
            {
                aiInputControlStateComponent.mAiControllerInputDelayTimer->Reset();
                inputStateComponent.mCurrentInputState[VirtualActionType::A_BUTTON] = VirtualActionInputState::TAPPED;
                aiInputControlStateComponent.mAiControllerInputProgressionStep++;
            }
        }
    }
}

void AiInputControlSystem::ResetInputState() const
{
    auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    
    std::for_each
    (
        inputStateComponent.mCurrentInputState.begin(),
        inputStateComponent.mCurrentInputState.end(),
        [] (std::pair<const VirtualActionType, VirtualActionInputState>& entry)     
        {
            entry.second = VirtualActionInputState::RELEASED;
        }
    );
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////