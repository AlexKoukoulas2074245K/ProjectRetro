//
//  GuiManagementSystem.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 07/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef GuiManagementSystem_h
#define GuiManagementSystem_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../GameConstants.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class GuiStateSingletonComponent;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class GuiManagementSystem final: public ecs::BaseSystem
{
public:
    GuiManagementSystem(ecs::World&);
    
    void VUpdateAssociatedComponents(const float dt) const override;
    
private:
    static const std::string TEXTBOX_CLICK_SFX_NAME;
    static const std::string BADGE_SFX_NAME;
    static const std::string KEY_ITEM_RECEIVED_SFX_NAME;
    static const std::string REGULAR_ITEM_RECEIVED_SFX_NAME;
    
    static float GUI_TILE_DEFAULT_SIZE;
    static float CHATBOX_BLINKING_CURSOR_COOLDOWN;
    static float CHATBOX_SCROLL_ANIM_COOLDOWN;
    
    void InitializeGuiState() const;
    void PopulateFontEntities(GuiStateSingletonComponent&) const;
    void UpdateChatbox(const ecs::EntityId textboxEntityId, const float dt) const;
    void UpdateChatboxNormal(const ecs::EntityId textboxEntityId, const float dt) const;
    void UpdateChatboxFilled(const ecs::EntityId textboxEntityId, const float dt) const;
    void UpdateChatboxScrollAnim1(const ecs::EntityId textboxEntityId, const float dt) const;
    void UpdateChatboxScrollAnim2(const ecs::EntityId textboxEntityId, const float dt) const;
    void UpdateChatboxParagraphEndDelay(const float dt) const;    
    void OnTextboxQueuedCharacterRemoval(const ecs::EntityId textboxEntityId) const;
    void OnItemReceived(const ecs::EntityId textboxEntityId, const ItemDiscoveryType) const;
    void UpdateCursoredTextbox(const ecs::EntityId textboxEntityId) const;

    bool DetectedKillSwitch(const ecs::EntityId textboxEntityId) const;
    bool DetectedFreeze(const ecs::EntityId textboxEntityId) const;
    bool DetectedFlowHook(const ecs::EntityId textboxEntityId) const;
    ItemDiscoveryType DetectedItemReceivedText(const ecs::EntityId textboxEntityId) const;    
    void StripSpecialHookStringFromQueuedText(const ecs::EntityId textboxEntityId) const;    
    void MoveTextboxCursor(const ecs::EntityId textboxEntityId, const Direction direction) const;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* GuiManagementSystem_h */
