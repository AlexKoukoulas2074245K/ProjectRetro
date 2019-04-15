//
//  InputStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef InputStateSingletonComponent_h
#define InputStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"

#include <SDL_keyboard.h>
#include <unordered_map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static constexpr int DEFAULT_KEY_COUNT = 512;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class VirtualActionType
{
    LEFT,
    RIGHT,
    UP,
    DOWN,
    A,
    B,
    START,
    SELECT,
    DEBUG_CAMERA_UP,
    DEBUG_CAMERA_DOWN,
    DEBUG_CAMERA_FORWARD,
    DEBUG_CAMERA_BACKWARD
};

enum class VirtualActionInputState
{
    RELEASED, PRESSED, TAPPED
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class InputStateSingletonComponent final: public ecs::IComponent
{
public:
    std::unordered_map<VirtualActionType, VirtualActionInputState> mCurrentInputState = 
    {
        { VirtualActionType::LEFT,                  VirtualActionInputState::RELEASED },
        { VirtualActionType::RIGHT,                 VirtualActionInputState::RELEASED },
        { VirtualActionType::UP,                    VirtualActionInputState::RELEASED },
        { VirtualActionType::DOWN,                  VirtualActionInputState::RELEASED },
        { VirtualActionType::A,                     VirtualActionInputState::RELEASED },
        { VirtualActionType::B,                     VirtualActionInputState::RELEASED },
        { VirtualActionType::START,                 VirtualActionInputState::RELEASED },
        { VirtualActionType::SELECT,                VirtualActionInputState::RELEASED },
        { VirtualActionType::DEBUG_CAMERA_UP,       VirtualActionInputState::RELEASED },
        { VirtualActionType::DEBUG_CAMERA_DOWN,     VirtualActionInputState::RELEASED },
        { VirtualActionType::DEBUG_CAMERA_FORWARD,  VirtualActionInputState::RELEASED },
        { VirtualActionType::DEBUG_CAMERA_BACKWARD, VirtualActionInputState::RELEASED },
    };

    std::unordered_map<SDL_Scancode, VirtualActionType> mKeybindings = 
    {
        { SDL_SCANCODE_LEFT,      VirtualActionType::LEFT },
        { SDL_SCANCODE_RIGHT,     VirtualActionType::RIGHT },
        { SDL_SCANCODE_UP,        VirtualActionType::UP },
        { SDL_SCANCODE_DOWN,      VirtualActionType::DOWN },
        { SDL_SCANCODE_Z,         VirtualActionType::A },
        { SDL_SCANCODE_X,         VirtualActionType::B },
        { SDL_SCANCODE_RETURN,    VirtualActionType::START },
        { SDL_SCANCODE_BACKSPACE, VirtualActionType::SELECT },
        { SDL_SCANCODE_Q,         VirtualActionType::DEBUG_CAMERA_UP },
        { SDL_SCANCODE_E,         VirtualActionType::DEBUG_CAMERA_DOWN },
        { SDL_SCANCODE_W,         VirtualActionType::DEBUG_CAMERA_FORWARD },
        { SDL_SCANCODE_S,         VirtualActionType::DEBUG_CAMERA_BACKWARD },
    };

    // Needed for tapped input checks 
    std::vector<unsigned char> mPreviousRawKeyboardState;
    
    bool mHasBeenConsumed = false;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* InputStateSingletonComponent_h */
