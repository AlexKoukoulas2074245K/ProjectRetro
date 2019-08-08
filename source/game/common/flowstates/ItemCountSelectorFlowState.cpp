//
//  ItemCountSelectorFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 08/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ItemMenuFlowState.h"
#include "ItemCountSelectorFlowState.h"
#include "TossItemFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/ItemCountSelectorStateComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 ItemCountSelectorFlowState::ITEM_COUNT_SELECTOR_TEXTBOX_POSITION = glm::vec3(0.51f, -0.1438f, -0.3f);
const glm::vec3 ItemCountSelectorFlowState::YES_NO_TEXTBOX_POSITION              = glm::vec3(0.481498629f, -0.065f, -0.4f);

const float ItemCountSelectorFlowState::IS_IT_OK_TO_TOSS_CHATBOX_Z = -0.4f;

const int ItemCountSelectorFlowState::ITEM_COUNT_SELECTOR_COLS = 5;
const int ItemCountSelectorFlowState::ITEM_COUNT_SELECTOR_ROWS = 3;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ItemCountSelectorFlowState::ItemCountSelectorFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& itemBagEntry         = playerStateComponent.mPlayerBag.at(playerStateComponent.mPreviousItemMenuItemOffset + playerStateComponent.mPreviousItemMenuCursorRow);    

    const auto itemCountSelectorEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CHATBOX, 
        ITEM_COUNT_SELECTOR_COLS,
        ITEM_COUNT_SELECTOR_ROWS,
        ITEM_COUNT_SELECTOR_TEXTBOX_POSITION.x,
        ITEM_COUNT_SELECTOR_TEXTBOX_POSITION.y,
        ITEM_COUNT_SELECTOR_TEXTBOX_POSITION.z,
        mWorld
    );

    auto itemCountSelectorStateComponent = std::make_unique<ItemCountSelectorStateComponent>();
    itemCountSelectorStateComponent->mSelectorMinValue     = 1;
    itemCountSelectorStateComponent->mSelectorCurrentValue = 1;
    itemCountSelectorStateComponent->mSelectorMaxValue     = itemBagEntry.mQuantity;
    mWorld.AddComponent<ItemCountSelectorStateComponent>(itemCountSelectorEntityId, std::move(itemCountSelectorStateComponent));

    DrawSelectorCount();
}

void ItemCountSelectorFlowState::VUpdate(const float)
{            
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // Item Count Selector is Active
    if (guiStateComponent.mActiveTextboxesStack.size() == 4)
    {
        UpdateItemCountSelectorTextbox();
    }
    // Is it ok to toss chatbox Active and Frozen
    else if 
    (
        guiStateComponent.mActiveTextboxesStack.size() == 5 && 
        guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN
    )
    {
        CreateYesNoTextbox(mWorld, YES_NO_TEXTBOX_POSITION);
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 6)
    {
        UpdateYesNoTextbox();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void ItemCountSelectorFlowState::UpdateItemCountSelectorTextbox()
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& itemSelectorCounterStateComponent = mWorld.GetComponent<ItemCountSelectorStateComponent>(GetActiveTextboxEntityId(mWorld));

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
        const auto& itemBagEntry = playerStateComponent.mPlayerBag.at(playerStateComponent.mPreviousItemMenuItemOffset + playerStateComponent.mPreviousItemMenuCursorRow);

        const auto isItOkToTossChatboxEntityId = CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, IS_IT_OK_TO_TOSS_CHATBOX_Z));
        QueueDialogForChatbox(isItOkToTossChatboxEntityId, "Is it OK to toss#" + itemBagEntry.mItemName.GetString() + "?+FREEZE", mWorld);
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        // Destroy Item Selector Textbox
        DestroyActiveTextbox(mWorld);

        // Destroy Use/Toss Textbox
        DestroyActiveTextbox(mWorld);

        CompleteAndTransitionTo<ItemMenuFlowState>();
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent))
    {
        if (++itemSelectorCounterStateComponent.mSelectorCurrentValue > itemSelectorCounterStateComponent.mSelectorMaxValue)
        {
            itemSelectorCounterStateComponent.mSelectorCurrentValue = itemSelectorCounterStateComponent.mSelectorMinValue;
        }

        DrawSelectorCount();

    }
    else if (IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent))
    {
        if (--itemSelectorCounterStateComponent.mSelectorCurrentValue < itemSelectorCounterStateComponent.mSelectorMinValue)
        {
            itemSelectorCounterStateComponent.mSelectorCurrentValue = itemSelectorCounterStateComponent.mSelectorMaxValue;
        }

        DrawSelectorCount();
    }
}

void ItemCountSelectorFlowState::UpdateYesNoTextbox()
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& cursorComponent     = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        // Yes Selected
        if (cursorComponent.mCursorRow == 0)
        {
            TransitionToTossItemsFlow();
        }
        // No Selected
        else if (cursorComponent.mCursorRow == 1)
        {
            CancelYesNoTossItem();
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        CancelYesNoTossItem();
    }
}

void ItemCountSelectorFlowState::CancelYesNoTossItem()
{
    // Destroy Yes/No textbox
    DestroyActiveTextbox(mWorld);

    // Destroy Is it Ok to toss Chatbox 
    DestroyActiveTextbox(mWorld);

    // Destroy Item Selector Textbox
    DestroyActiveTextbox(mWorld);

    // Destroy Use/Toss Textbox
    DestroyActiveTextbox(mWorld);

    CompleteAndTransitionTo<ItemMenuFlowState>();
}

void ItemCountSelectorFlowState::TransitionToTossItemsFlow()
{
    // Destroy Yes/No textbox
    DestroyActiveTextbox(mWorld);

    // Destroy Is it Ok to toss Chatbox 
    DestroyActiveTextbox(mWorld);

    const auto& itemSelectorStateComponent = mWorld.GetComponent<ItemCountSelectorStateComponent>(GetActiveTextboxEntityId(mWorld));
    auto& playerStateComponent             = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    playerStateComponent.mQuantityOfItemsToToss = itemSelectorStateComponent.mSelectorCurrentValue;
    
    CompleteAndTransitionTo<TossItemFlowState>();
}

void ItemCountSelectorFlowState::DrawSelectorCount() const
{
    const auto itemSelectorCountTextboxEntityId = GetActiveTextboxEntityId(mWorld);
    auto& itemSelectorCounterStateComponent     = mWorld.GetComponent<ItemCountSelectorStateComponent>(itemSelectorCountTextboxEntityId);

    if (itemSelectorCounterStateComponent.mSelectorCurrentValue > 9)
    {
        WriteTextAtTextboxCoords(itemSelectorCountTextboxEntityId, "*" + std::to_string(itemSelectorCounterStateComponent.mSelectorCurrentValue), 1, 1, mWorld);
    }
    else
    {
        WriteTextAtTextboxCoords(itemSelectorCountTextboxEntityId, "*0" + std::to_string(itemSelectorCounterStateComponent.mSelectorCurrentValue), 1, 1, mWorld);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
