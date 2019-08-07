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
    LEFT_ARROW,
    RIGHT_ARROW,
    UP_ARROW,
    DOWN_ARROW,
    A_BUTTON,
    B_BUTTON,
    START_BUTTON,
    SELECT_BUTTON,
    DEBUG_CAMERA_UP,
    DEBUG_CAMERA_DOWN,
    DEBUG_CAMERA_FORWARD,
    DEBUG_CAMERA_BACKWARD,
    DEBUG_SPEED_UP
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
        { VirtualActionType::LEFT_ARROW,                  VirtualActionInputState::RELEASED },
        { VirtualActionType::RIGHT_ARROW,                 VirtualActionInputState::RELEASED },
        { VirtualActionType::UP_ARROW,                    VirtualActionInputState::RELEASED },
        { VirtualActionType::DOWN_ARROW,                  VirtualActionInputState::RELEASED },
        { VirtualActionType::A_BUTTON,                    VirtualActionInputState::RELEASED },
        { VirtualActionType::B_BUTTON,                    VirtualActionInputState::RELEASED },
        { VirtualActionType::START_BUTTON,                VirtualActionInputState::RELEASED },
        { VirtualActionType::SELECT_BUTTON,               VirtualActionInputState::RELEASED },
        { VirtualActionType::DEBUG_CAMERA_UP,             VirtualActionInputState::RELEASED },
        { VirtualActionType::DEBUG_CAMERA_DOWN,           VirtualActionInputState::RELEASED },
        { VirtualActionType::DEBUG_CAMERA_FORWARD,        VirtualActionInputState::RELEASED },
        { VirtualActionType::DEBUG_CAMERA_BACKWARD,       VirtualActionInputState::RELEASED },
        { VirtualActionType::DEBUG_SPEED_UP,              VirtualActionInputState::RELEASED },
    };

    std::unordered_map<SDL_Scancode, VirtualActionType> mKeybindings = 
    {
        { SDL_SCANCODE_LEFT,      VirtualActionType::LEFT_ARROW },
        { SDL_SCANCODE_RIGHT,     VirtualActionType::RIGHT_ARROW },
        { SDL_SCANCODE_UP,        VirtualActionType::UP_ARROW },
        { SDL_SCANCODE_DOWN,      VirtualActionType::DOWN_ARROW },
        { SDL_SCANCODE_Z,         VirtualActionType::A_BUTTON },
        { SDL_SCANCODE_X,         VirtualActionType::B_BUTTON },
        { SDL_SCANCODE_RETURN,    VirtualActionType::START_BUTTON },
        { SDL_SCANCODE_BACKSPACE, VirtualActionType::SELECT_BUTTON },
        { SDL_SCANCODE_Q,         VirtualActionType::DEBUG_CAMERA_UP },
        { SDL_SCANCODE_E,         VirtualActionType::DEBUG_CAMERA_DOWN },
        { SDL_SCANCODE_W,         VirtualActionType::DEBUG_CAMERA_FORWARD },
        { SDL_SCANCODE_S,         VirtualActionType::DEBUG_CAMERA_BACKWARD },
        { SDL_SCANCODE_SPACE,     VirtualActionType::DEBUG_SPEED_UP },
    };

    // Needed for tapped input checks 
    std::vector<unsigned char> mPreviousRawKeyboardState;
    
    bool mHasBeenConsumed = false;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* InputStateSingletonComponent_h */
