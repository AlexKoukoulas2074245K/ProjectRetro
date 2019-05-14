//
//  GuiStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef GuiStateSingletonComponent_h
#define GuiStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../utils/Timer.h"
#include "../../ECS.h"
#include "../../rendering/components/RenderableComponent.h"

#include <memory>
#include <stack>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string GUI_COMPONENTS_MODEL_NAME = "camera_facing_quad";

const int GUI_ATLAS_COLS = 16;
const int GUI_ATLAS_ROWS = 16;

const float DEFAULT_CHATBOX_CHAR_COOLDOWN = 0.05f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class ChatboxDisplayState
{
    NORMAL, FILLED, SCROLL_ANIM_PHASE_1, SCROLL_ANIM_PHASE_2, PARAGRAPH_END_DELAY
};

enum class ChatboxContentEndState
{
    NORMAL, PARAGRAPH_END, DIALOG_END
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class GuiStateSingletonComponent final: public ecs::IComponent
{
public:
    float mGlobalGuiTileWidth                         = 0.0f;
    float mGlobalGuiTileHeight                        = 0.0f;
    float mChatboxCharCooldown                        = DEFAULT_CHATBOX_CHAR_COOLDOWN;
    std::unique_ptr<Timer> mActiveChatboxTimer        = nullptr;
    ChatboxDisplayState mActiveChatboxDisplayState    = ChatboxDisplayState::NORMAL;
    ChatboxContentEndState mActiveChatboxContentState = ChatboxContentEndState::NORMAL;
    std::stack<ecs::EntityId> mActiveTextboxesStack;
    std::unordered_map<char, ResourceId> mFontEntities;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* GuiStateSingletonComponent_h */
