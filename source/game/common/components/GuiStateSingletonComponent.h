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

static const std::string GUI_COMPONENTS_MODEL_NAME = "camera_facing_quad_hud_sub_atlas";

const int GUI_ATLAS_COLS = 16;
const int GUI_ATLAS_ROWS = 16;

const float DEFAULT_CHATBOX_CHAR_COOLDOWN = 0.05f;
const float MORE_ITEMS_CURSOR_TIMER_DELAY = 0.35f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class ChatboxDisplayState
{
    NORMAL, FILLED, SCROLL_ANIM_PHASE_1, SCROLL_ANIM_PHASE_2, PARAGRAPH_END_DELAY, FROZEN
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
    std::stack<ecs::EntityId> mActiveTextboxesStack;
    std::unordered_map<char, ResourceId> mFontEntities;
    std::unique_ptr<Timer> mActiveChatboxTimer        = nullptr;
    std::unique_ptr<Timer> mMoreItemsCursorTimer      = nullptr;
    float mGlobalGuiTileWidth                         = 0.0f;
    float mGlobalGuiTileHeight                        = 0.0f;
    float mChatboxCharCooldown                        = DEFAULT_CHATBOX_CHAR_COOLDOWN;
    ChatboxDisplayState mActiveChatboxDisplayState    = ChatboxDisplayState::NORMAL;
    ChatboxContentEndState mActiveChatboxContentState = ChatboxContentEndState::NORMAL;
    bool mShouldDisplayIndicationForMoreItems         = false;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* GuiStateSingletonComponent_h */
