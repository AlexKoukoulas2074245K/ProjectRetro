//
//  PokeMartBuyDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokeMartBuyDialogOverworldFlowState.h"
#include "PokeMartMenuSelectionOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/ItemMenuStateComponent.h"
#include "../components/MarketStocksSingletonComponent.h"
#include "../components/PokeMartDialogStateSingletonComponent.h"
#include "../utils/PokemonItemsUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/TownMapLocationDataSingletonComponent.h"
#include "../../overworld/utils/OverworldUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PokeMartBuyDialogOverworldFlowState::TEXTBOX_CLICK_SFX_NAME = "general/textbox_click";
const float PokeMartBuyDialogOverworldFlowState::OVERLAID_CHATBOX_Z = -0.05f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokeMartBuyDialogOverworldFlowState::PokeMartBuyDialogOverworldFlowState(ecs::World& world)
    : BaseFlowState(world)
    , mItemQuantityTextboxEntityId(ecs::NULL_ENTITY_ID)
    , mSelectedItemName()
    , mSelectedQuantity(0)
    , mCancellingDialog(false)
{
    QueueDialogForChatbox(CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, OVERLAID_CHATBOX_Z)), "Take your time.+FREEZE", mWorld);
    
    auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::NORMAL;    
}

void PokeMartBuyDialogOverworldFlowState::VUpdate(const float dt)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (mCancellingDialog && guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        auto& pokemartDialogComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();

        // Destroy is there anything I can do chatbox
        DestroyActiveTextbox(mWorld);

        // Destroy menu selection dialog
        DestroyActiveTextbox(mWorld);
        pokemartDialogComponent.mMenuTextboxEntityId = ecs::NULL_ENTITY_ID;

        // Destroy may I help you chatbox
        DestroyActiveTextbox(mWorld);

        const auto activeChatboxEntityId = CreateChatbox(mWorld);
        WriteTextAtTextboxCoords(activeChatboxEntityId, "Is there anything", 1, 2, mWorld);
        WriteTextAtTextboxCoords(activeChatboxEntityId, "else I can do?", 1, 4, mWorld);

        // Destroy money textbox
        DestroyGenericOrBareTextbox(pokemartDialogComponent.mMoneyTextboxEntityId, mWorld);
        pokemartDialogComponent.mMoneyTextboxEntityId = ecs::NULL_ENTITY_ID;

        CompleteAndTransitionTo<PokeMartMenuSelectionOverworldFlowState>();
    }
    // Item menu not yet created
    else if
    (
        guiStateComponent.mActiveTextboxesStack.size() == 3 &&
        guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN
    )
    {
        // Create and populate item menu
        const auto& activeLevelComponent  = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
        const auto& marketStocksComponent = mWorld.GetSingletonComponent<MarketStocksSingletonComponent>();
        const auto& itemsInStock = marketStocksComponent.mMarketStocks.at(GetLevelOwnerNameOfLocation(activeLevelComponent.mActiveLevelNameId, mWorld));
        
        const auto itemMenuEntityId = CreateItemMenu
        (
            mWorld,
            itemsInStock.size()
        );
        
        auto& itemMenuComponent = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);
        itemMenuComponent.mItemMenuOffsetFromStart = 0;
        itemMenuComponent.mPreviousCursorRow       = 0;
        
        DisplayItemsInMenuForCurrentOffset();
    }
    // Item menu active
    else if 
    (
        guiStateComponent.mActiveTextboxesStack.size() == 4
    )
    {
        UpdateItemMenu(dt);
    }
    // Item quantity selection active
    else if 
    (
        guiStateComponent.mActiveTextboxesStack.size() == 5
    )
    {
        UpdateItemQuantityMenu(dt);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PokeMartBuyDialogOverworldFlowState::UpdateItemMenu(const float dt)
{    
    const auto& activeLevelComponent  = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& marketStocksComponent = mWorld.GetSingletonComponent<MarketStocksSingletonComponent>();
    const auto& itemsInStock          = marketStocksComponent.mMarketStocks.at(GetLevelOwnerNameOfLocation(activeLevelComponent.mActiveLevelNameId, mWorld));
    const auto& inputStateComponent   = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto itemMenuEntityId       = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent       = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);
    auto& itemMenuStateComponent      = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);
    auto& guiStateComponent           = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    guiStateComponent.mMoreItemsCursorTimer->Update(dt);
    if (guiStateComponent.mMoreItemsCursorTimer->HasTicked())
    {
        guiStateComponent.mMoreItemsCursorTimer->Reset();

        if (itemMenuStateComponent.mItemMenuOffsetFromStart + 4 < static_cast<int>(itemsInStock.size()))
        {
            guiStateComponent.mShouldDisplayIndicationForMoreItems = !guiStateComponent.mShouldDisplayIndicationForMoreItems;

            if (guiStateComponent.mShouldDisplayIndicationForMoreItems)
            {
                WriteCharAtTextboxCoords(GetActiveTextboxEntityId(mWorld), '|', 14, 9, mWorld);
            }
            else
            {
                DeleteCharAtTextboxCoords(GetActiveTextboxEntityId(mWorld), 14, 9, mWorld);
            }
        }
    }

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {        
        const auto& itemName  = itemsInStock.at(itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow);
        const auto& itemStats = GetItemStats(itemName, mWorld);

        if (itemName == CANCEL_ITEM_NAME)
        {
            CancelDialog();
            return;
        }
        else
        {
            mSelectedQuantity = 1;
            mSelectedItemName = itemName;
            mItemQuantityTextboxEntityId = CreatePokeMartItemQuantityTextbox(mWorld, itemStats.mPrice);
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        CancelDialog();
        return;
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent))
    {
        if (cursorComponent.mCursorRow == 0 && itemMenuStateComponent.mPreviousCursorRow == 0)
        {
            itemMenuStateComponent.mItemMenuOffsetFromStart--;
            if (itemMenuStateComponent.mItemMenuOffsetFromStart <= 0)
            {
                itemMenuStateComponent.mItemMenuOffsetFromStart = 0;
            }

            RedrawItemMenu();
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent))
    {
        if (cursorComponent.mCursorRow == 2 && itemMenuStateComponent.mPreviousCursorRow == 2)
        {
            itemMenuStateComponent.mItemMenuOffsetFromStart++;
            if
            (
                itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow >=
                static_cast<int>(itemsInStock.size()) - 1
            )
            {
                itemMenuStateComponent.mItemMenuOffsetFromStart =
                 static_cast<int>(itemsInStock.size()) - 1 - cursorComponent.mCursorRow;
            }

            RedrawItemMenu();
        }
    }        

    SaveLastFramesCursorRow();
}

void PokeMartBuyDialogOverworldFlowState::UpdateItemQuantityMenu(const float)
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();

    if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent))
    {
        
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent))
    {
    }
}

void PokeMartBuyDialogOverworldFlowState::CancelDialog()
{
    // Destroy item menu textbox
    DestroyActiveTextbox(mWorld);

    // Destroy take your time chatbox
    DestroyActiveTextbox(mWorld);

    QueueDialogForChatbox
    (
        CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, OVERLAID_CHATBOX_Z)),
        "Is there anything#else I can do?+FREEZE",
        mWorld
    );

    mCancellingDialog = true;
}

void PokeMartBuyDialogOverworldFlowState::RedrawItemMenu()
{
    const auto& activeLevelComponent  = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& marketStocksComponent = mWorld.GetSingletonComponent<MarketStocksSingletonComponent>();
    const auto& itemsInStock          = marketStocksComponent.mMarketStocks.at(GetLevelOwnerNameOfLocation(activeLevelComponent.mActiveLevelNameId, mWorld));    
    const auto itemMenuEntityId       = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent       = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);
    auto& itemMenuStateComponent      = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);

    const auto cursorRow  = cursorComponent.mCursorRow;
    const auto itemOffset = itemMenuStateComponent.mItemMenuOffsetFromStart;

    DestroyActiveTextbox(mWorld);
    CreateItemMenu(mWorld, itemsInStock.size(), cursorRow, itemOffset);
    DisplayItemsInMenuForCurrentOffset();
}

void PokeMartBuyDialogOverworldFlowState::SaveLastFramesCursorRow()
{
    const auto activeTextboxEntityId  = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent = mWorld.GetComponent<CursorComponent>(activeTextboxEntityId);
    auto& itemMenuStateComponent = mWorld.GetComponent<ItemMenuStateComponent>(activeTextboxEntityId);

    itemMenuStateComponent.mPreviousCursorRow = cursorComponent.mCursorRow;    
}

void PokeMartBuyDialogOverworldFlowState::DisplayItemsInMenuForCurrentOffset() const
{
    const auto& activeLevelComponent  = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& itemMenuEntityId      = GetActiveTextboxEntityId(mWorld);
    const auto& itemMenuComponent     = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);
    const auto& marketStocksComponent = mWorld.GetSingletonComponent<MarketStocksSingletonComponent>();
    const auto& itemsInStock          = marketStocksComponent.mMarketStocks.at(GetLevelOwnerNameOfLocation(activeLevelComponent.mActiveLevelNameId, mWorld));

    auto cursorRowIndex = 0U;
    for
    (
        auto i = itemMenuComponent.mItemMenuOffsetFromStart;
        i < math::Min(itemMenuComponent.mItemMenuOffsetFromStart + 4, static_cast<int>(itemsInStock.size()));
        ++i
    )
    {
        const auto& itemNameId = itemsInStock.at(i);
        const auto& itemStats  = GetItemStats(itemNameId, mWorld);
        const auto& itemPriceString = "$" + std::to_string(itemStats.mPrice);
        
        WriteTextAtTextboxCoords(itemMenuEntityId, itemNameId.GetString(), 2, 2 + cursorRowIndex * 2, mWorld);

        if (itemStats.mName != StringId("CANCEL"))
        {
            WriteTextAtTextboxCoords(itemMenuEntityId, itemPriceString, 14 - itemPriceString.size(), 3 + cursorRowIndex * 2, mWorld);
        }
        
        cursorRowIndex++;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
