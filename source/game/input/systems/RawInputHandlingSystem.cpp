//
//  RawInputHandlingSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "RawInputHandlingSystem.h"
#include "../components/InputStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

RawInputHandlingSystem::RawInputHandlingSystem(ecs::World& world)
    : BaseSystem(world)
{
    auto inputStateComponent = std::make_unique<InputStateSingletonComponent>();
    inputStateComponent->mPreviousRawKeyboardState.resize(DEFAULT_KEY_COUNT, 0);

    mWorld.SetSingletonComponent<InputStateSingletonComponent>(std::move(inputStateComponent));
}

void RawInputHandlingSystem::VUpdateAssociatedComponents(const float) const
{   
    auto keyboardStateLength         = 0;
    const auto* currentKeyboardState = SDL_GetKeyboardState(&keyboardStateLength);
    auto& inputStateComponent        = mWorld.GetSingletonComponent<InputStateSingletonComponent>();

    for (const auto& keybindingEntry: inputStateComponent.mKeybindings)
    {
        const auto& sdlScancode         = keybindingEntry.first;
        const auto& mappedVirtualAction = keybindingEntry.second;

        // Key down this frame but not last frame (tap)
        if (currentKeyboardState[sdlScancode] && !inputStateComponent.mPreviousRawKeyboardState[sdlScancode])
        {
            inputStateComponent.mCurrentInputState[mappedVirtualAction] = VirtualActionInputState::TAPPED;
        }
        // Key down this frame and last frame (pressed)
        else if (currentKeyboardState[sdlScancode] && inputStateComponent.mPreviousRawKeyboardState[sdlScancode])
        {
            inputStateComponent.mCurrentInputState[mappedVirtualAction] = VirtualActionInputState::PRESSED;
        }
        // Key up this frame, but down last frame (released)
        else
        {
            inputStateComponent.mCurrentInputState[mappedVirtualAction] = VirtualActionInputState::RELEASED;
        }
    }

    inputStateComponent.mPreviousRawKeyboardState.assign(currentKeyboardState, currentKeyboardState + keyboardStateLength);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////