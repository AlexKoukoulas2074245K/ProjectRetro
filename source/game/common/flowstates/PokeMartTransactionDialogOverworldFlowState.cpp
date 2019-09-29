//
//  PokeMartTransactionDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokeMartTransactionDialogOverworldFlowState.h"
#include "PokeMartMenuSelectionOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/ItemMenuStateComponent.h"
#include "../components/MarketStocksSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokeMartDialogStateSingletonComponent.h"
#include "../utils/PokemonItemsUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/TownMapLocationDataSingletonComponent.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PokeMartTransactionDialogOverworldFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.065f, -0.6f);

const std::string PokeMartTransactionDialogOverworldFlowState::TEXTBOX_CLICK_SFX_NAME = "general/textbox_click";
const std::string PokeMartTransactionDialogOverworldFlowState::PURCHASE_SFX_NAME      = "general/purchase";

const float PokeMartTransactionDialogOverworldFlowState::FIRST_OVERLAID_CHATBOX_Z  = -0.05f;
const float PokeMartTransactionDialogOverworldFlowState::SECOND_OVERLAID_CHATBOX_Z = -0.15f;

const int PokeMartTransactionDialogOverworldFlowState::MIN_ITEM_QUANTITY = 1;
const int PokeMartTransactionDialogOverworldFlowState::MAX_ITEM_QUANTITY = 99;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokeMartTransactionDialogOverworldFlowState::PokeMartTransactionDialogOverworldFlowState(ecs::World& world)
    : BaseFlowState(world)
    , mSelectedItemName()
    , mBuyDialogState(BuyDialogState::INTRO)
    , mBagItemIndex(0)
    , mSelectedQuantity(0)
{
    const auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
    const auto promptDialog = pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY ? "Take your time.+FREEZE" : "What would you#like to sell?+FREEZE";
    QueueDialogForChatbox(CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, FIRST_OVERLAID_CHATBOX_Z)), promptDialog, mWorld);
    
    auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::NORMAL;    
}

void PokeMartTransactionDialogOverworldFlowState::VUpdate(const float dt)
{
    switch (mBuyDialogState)
    {
        case BuyDialogState::INTRO:               UpdateIntroFlow(); break;
        case BuyDialogState::ITEM_MENU:           UpdateItemMenu(dt); break;
        case BuyDialogState::ITEM_QUANTITY:       UpdateItemQuantityMenu(); break;
        case BuyDialogState::CONFIRMATION:        UpdateConfirmationDialog(); break;
        case BuyDialogState::CONFIRMATION_YES_NO: UpdateConfirmationYesNoDialog(); break;
        case BuyDialogState::NOT_ENOUGH_MONEY:    UpdateNotEnoughMoneyFlow(); break;
        case BuyDialogState::UNIQUE_ITEM_SALE:    UpdateUniqueItemSaleFlow(); break;
        case BuyDialogState::SUCCESSFUL_PURCHASE: UpdateSuccessfulPurchaseFlow(); break;
        case BuyDialogState::CANCELLING:          UpdateCancellingFlow(); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PokeMartTransactionDialogOverworldFlowState::UpdateIntroFlow()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        CreateAndPopulateItemMenu();
    }
}

void PokeMartTransactionDialogOverworldFlowState::UpdateItemMenu(const float dt)
{    
    const auto& activeLevelComponent         = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& marketStocksComponent        = mWorld.GetSingletonComponent<MarketStocksSingletonComponent>();
    const auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
    const auto& playerStateComponent         = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& guiStateComponent                  = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& itemsInStock                 = marketStocksComponent.mMarketStocks.at(GetLevelOwnerNameOfLocation(activeLevelComponent.mActiveLevelNameId, mWorld));
    const auto& inputStateComponent          = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto itemMenuEntityId              = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent              = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);
    auto& itemMenuStateComponent             = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);

    guiStateComponent.mMoreItemsCursorTimer->Update(dt);
    if (guiStateComponent.mMoreItemsCursorTimer->HasTicked())
    {
        guiStateComponent.mMoreItemsCursorTimer->Reset();

        const auto itemCount = pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY ? itemsInStock.size() : playerStateComponent.mPlayerBag.size();
        if (itemMenuStateComponent.mItemMenuOffsetFromStart + 4 < static_cast<int>(itemCount))
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
        
        const auto& itemName  = pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY ?
            itemsInStock.at(itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow) :
            playerStateComponent.mPlayerBag.at(itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow).mItemName;
        
        if (pokeMartDialogStateComponent.mTransactionType == TransactionType::SELL)
        {
            mBagItemIndex = itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow;
        }
        
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
            
            if (pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY)
            {
                CreatePokeMartItemQuantityTextbox(mWorld, mSelectedQuantity, itemStats.mPrice);
                mBuyDialogState = BuyDialogState::ITEM_QUANTITY;
            }
            else
            {
                if (itemStats.mUnique)
                {
                    QueueDialogForChatbox
                    (
                        CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, SECOND_OVERLAID_CHATBOX_Z)),
                        "I can't put a#price on that.#+END",
                        mWorld
                    );
                    
                    guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::NORMAL;
                    mBuyDialogState = BuyDialogState::UNIQUE_ITEM_SALE;
                }
                else
                {
                    CreatePokeMartItemQuantityTextbox(mWorld, mSelectedQuantity, itemStats.mPrice/2);
                    mBuyDialogState = BuyDialogState::ITEM_QUANTITY;
                }
            }
            
            return;
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
            
            if (pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY)
            {
                if
                (
                    itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow >=
                    static_cast<int>(itemsInStock.size()) - 1
                )
                {
                    itemMenuStateComponent.mItemMenuOffsetFromStart =
                    static_cast<int>(itemsInStock.size()) - 1 - cursorComponent.mCursorRow;
                }
            }
            else
            {
                if
                (
                    itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow >=
                    static_cast<int>(playerStateComponent.mPlayerBag.size()) - 1
                )
                {
                    itemMenuStateComponent.mItemMenuOffsetFromStart =
                    static_cast<int>(playerStateComponent.mPlayerBag.size()) - 1 - cursorComponent.mCursorRow;
                }
            }

            RedrawItemMenu();
        }
    }        

    SaveLastFramesCursorRow();
}

void PokeMartTransactionDialogOverworldFlowState::UpdateItemQuantityMenu()
{
    const auto& inputStateComponent          = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
    const auto& playerStateComponent         = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    const auto& itemStats = GetItemStats(mSelectedItemName, mWorld);
    
    auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        if (pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY)
        {
            QueueDialogForChatbox
            (
                CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, SECOND_OVERLAID_CHATBOX_Z)),
                mSelectedItemName.GetString() + "?#That will be#$" + std::to_string(itemStats.mPrice * mSelectedQuantity) + ". OK?+FREEZE",
                mWorld
            );
        }
        else
        {
            QueueDialogForChatbox
            (
                CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, SECOND_OVERLAID_CHATBOX_Z)),
                "I can pay you#$" + std::to_string((itemStats.mPrice/2) * mSelectedQuantity) + " for that.+FREEZE",
                mWorld
            );
        }
        
        mBuyDialogState = BuyDialogState::CONFIRMATION;
        guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::NORMAL;
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        DestroyActiveTextbox(mWorld);
        mBuyDialogState = BuyDialogState::ITEM_MENU;
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent))
    {
        if (pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY)
        {
            if (++mSelectedQuantity > MAX_ITEM_QUANTITY)
            {
                mSelectedQuantity = MIN_ITEM_QUANTITY;
            }
        }
        else
        {
            const auto& itemQuantity = playerStateComponent.mPlayerBag.at(mBagItemIndex).mQuantity;
            if (++mSelectedQuantity > itemQuantity)
            {
                mSelectedQuantity = MIN_ITEM_QUANTITY;
            }
        }
        
        DestroyActiveTextbox(mWorld);
        
        if (pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY)
        {
            CreatePokeMartItemQuantityTextbox(mWorld, mSelectedQuantity, itemStats.mPrice * mSelectedQuantity);
        }
        else
        {
            CreatePokeMartItemQuantityTextbox(mWorld, mSelectedQuantity, (itemStats.mPrice/2) * mSelectedQuantity);
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent))
    {
        if (pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY)
        {
            if (--mSelectedQuantity < MIN_ITEM_QUANTITY)
            {
                mSelectedQuantity = MAX_ITEM_QUANTITY;
            }
        }
        else
        {
            const auto& itemQuantity = playerStateComponent.mPlayerBag.at(mBagItemIndex).mQuantity;
            if (--mSelectedQuantity < MIN_ITEM_QUANTITY)
            {
                mSelectedQuantity = itemQuantity;
            }
        }
        
        DestroyActiveTextbox(mWorld);
        
        if (pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY)
        {
            CreatePokeMartItemQuantityTextbox(mWorld, mSelectedQuantity, itemStats.mPrice * mSelectedQuantity);
        }
        else
        {
            CreatePokeMartItemQuantityTextbox(mWorld, mSelectedQuantity, (itemStats.mPrice/2) * mSelectedQuantity);
        }
    }
}

void PokeMartTransactionDialogOverworldFlowState::UpdateConfirmationDialog()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        mBuyDialogState = BuyDialogState::CONFIRMATION_YES_NO;
        CreateYesNoTextbox(mWorld, YES_NO_TEXTBOX_POSITION);
    }
}

void PokeMartTransactionDialogOverworldFlowState::UpdateConfirmationYesNoDialog()
{
    const auto& inputStateComponent          = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
    const auto& cursorComponent              = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
    const auto& itemStats                    = GetItemStats(mSelectedItemName, mWorld);
    auto& playerStateComponent               = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    const auto yesNoTextboxCursorRow = cursorComponent.mCursorRow;
    
    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        // Yes Selected
        if (yesNoTextboxCursorRow == 0)
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);
            
            // Destroy confirmation chatbox
            DestroyActiveTextbox(mWorld);
            
            // Buying flow
            if (pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY)
            {
                // Not enough money
                if (playerStateComponent.mPokeDollarCredits < mSelectedQuantity * itemStats.mPrice)
                {
                    QueueDialogForChatbox
                    (
                        CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, SECOND_OVERLAID_CHATBOX_Z)),
                        "You don't have#enough money.#+END",
                        mWorld
                    );
                    
                    mBuyDialogState = BuyDialogState::NOT_ENOUGH_MONEY;
                }
                else
                {
                    // Perform transaction
                    playerStateComponent.mPokeDollarCredits -= mSelectedQuantity * itemStats.mPrice;
                    AddItemToBag(mSelectedItemName, mWorld, mSelectedQuantity);
                    
                    // Update money textbox
                    auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
                    DestroyGenericOrBareTextbox(pokeMartDialogStateComponent.mMoneyTextboxEntityId, mWorld);
                    pokeMartDialogStateComponent.mMoneyTextboxEntityId = CreatePokeMartMoneyTextbox(mWorld);
                    
                    SoundService::GetInstance().PlaySfx(PURCHASE_SFX_NAME);
                    QueueDialogForChatbox
                    (
                        CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, SECOND_OVERLAID_CHATBOX_Z)),
                        "Here you are!#Thank you!#+END",
                        mWorld
                    );
                    
                    mBuyDialogState = BuyDialogState::SUCCESSFUL_PURCHASE;
                }
            }
            // Sell flow
            else
            {
                // Perform transaction
                playerStateComponent.mPokeDollarCredits += mSelectedQuantity * (itemStats.mPrice/2);
                RemoveItemFromBag(mSelectedItemName, mWorld, mSelectedQuantity);
                
                // Update money textbox
                auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
                DestroyGenericOrBareTextbox(pokeMartDialogStateComponent.mMoneyTextboxEntityId, mWorld);
                pokeMartDialogStateComponent.mMoneyTextboxEntityId = CreatePokeMartMoneyTextbox(mWorld);
                
                SoundService::GetInstance().PlaySfx(PURCHASE_SFX_NAME);
                
                // Destroy Item quantity textbox
                DestroyActiveTextbox(mWorld);
                
                // Destroy Item Menu
                DestroyActiveTextbox(mWorld);
                
                CreateAndPopulateItemMenu();
            }
        }
        // No Selected
        else if (yesNoTextboxCursorRow == 1)
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);
            
            // Destroy confirmation chatbox
            DestroyActiveTextbox(mWorld);
            
            // Destroy Item quantity textbox
            DestroyActiveTextbox(mWorld);
            
            mBuyDialogState = BuyDialogState::ITEM_MENU;
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        // Destroy Yes/No textbox
        DestroyActiveTextbox(mWorld);
        
        // Destroy confirmation chatbox
        DestroyActiveTextbox(mWorld);
        
        // Destroy Item quantity textbox
        DestroyActiveTextbox(mWorld);
        
        mBuyDialogState = BuyDialogState::ITEM_MENU;
        
        SoundService::GetInstance().PlaySfx(TEXTBOX_CLICK_SFX_NAME);
    }
}

void PokeMartTransactionDialogOverworldFlowState::UpdateNotEnoughMoneyFlow()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 5)
    {
        // Destroy Item Quantity textbox
        DestroyActiveTextbox(mWorld);
        
        CancelDialog();
    }
}

void PokeMartTransactionDialogOverworldFlowState::UpdateUniqueItemSaleFlow()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 4)
    {
        mBuyDialogState = BuyDialogState::ITEM_MENU;
    }
}

void PokeMartTransactionDialogOverworldFlowState::UpdateSuccessfulPurchaseFlow()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 5)
    {
        // Destroy Item Quantity textbox
        DestroyActiveTextbox(mWorld);
        
        mBuyDialogState = BuyDialogState::ITEM_MENU;
    }
}

void PokeMartTransactionDialogOverworldFlowState::UpdateCancellingFlow()
{
    auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
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
}

void PokeMartTransactionDialogOverworldFlowState::CancelDialog()
{
    // Destroy item menu textbox
    DestroyActiveTextbox(mWorld);

    // Destroy take your time chatbox
    DestroyActiveTextbox(mWorld);

    QueueDialogForChatbox
    (
        CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, FIRST_OVERLAID_CHATBOX_Z)),
        "Is there anything#else I can do?+FREEZE",
        mWorld
    );

    mBuyDialogState = BuyDialogState::CANCELLING;
}

void PokeMartTransactionDialogOverworldFlowState::CreateAndPopulateItemMenu()
{
    // Create and populate item menu
    const auto& activeLevelComponent         = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& marketStocksComponent        = mWorld.GetSingletonComponent<MarketStocksSingletonComponent>();
    const auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
    const auto& playerStateComponent         = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& itemsInStock                 = marketStocksComponent.mMarketStocks.at(GetLevelOwnerNameOfLocation(activeLevelComponent.mActiveLevelNameId, mWorld));
    
    const auto itemMenuEntityId = CreateItemMenu(mWorld, pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY ? itemsInStock.size() : playerStateComponent.mPlayerBag.size());
    
    auto& itemMenuComponent = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);
    itemMenuComponent.mItemMenuOffsetFromStart = 0;
    itemMenuComponent.mPreviousCursorRow       = 0;
    
    DisplayItemsInMenuForCurrentOffset();
    
    mBuyDialogState = BuyDialogState::ITEM_MENU;
}

void PokeMartTransactionDialogOverworldFlowState::RedrawItemMenu()
{
    const auto& activeLevelComponent         = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& marketStocksComponent        = mWorld.GetSingletonComponent<MarketStocksSingletonComponent>();
    const auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
    const auto& playerStateComponent         = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& itemsInStock                 = marketStocksComponent.mMarketStocks.at(GetLevelOwnerNameOfLocation(activeLevelComponent.mActiveLevelNameId, mWorld));
    const auto itemMenuEntityId              = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent              = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);
    auto& itemMenuStateComponent             = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);

    const auto cursorRow  = cursorComponent.mCursorRow;
    const auto itemOffset = itemMenuStateComponent.mItemMenuOffsetFromStart;

    DestroyActiveTextbox(mWorld);
    CreateItemMenu(mWorld, pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY ? itemsInStock.size() : playerStateComponent.mPlayerBag.size(), cursorRow, itemOffset);
    DisplayItemsInMenuForCurrentOffset();
}

void PokeMartTransactionDialogOverworldFlowState::SaveLastFramesCursorRow()
{
    const auto activeTextboxEntityId  = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent = mWorld.GetComponent<CursorComponent>(activeTextboxEntityId);
    auto& itemMenuStateComponent = mWorld.GetComponent<ItemMenuStateComponent>(activeTextboxEntityId);

    itemMenuStateComponent.mPreviousCursorRow = cursorComponent.mCursorRow;    
}

void PokeMartTransactionDialogOverworldFlowState::DisplayItemsInMenuForCurrentOffset() const
{
    const auto& activeLevelComponent         = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& marketStocksComponent        = mWorld.GetSingletonComponent<MarketStocksSingletonComponent>();
    const auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
    const auto& playerStateComponent         = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& itemMenuEntityId             = GetActiveTextboxEntityId(mWorld);
    const auto& itemMenuComponent            = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);
    const auto& itemsInStock                 = marketStocksComponent.mMarketStocks.at(GetLevelOwnerNameOfLocation(activeLevelComponent.mActiveLevelNameId, mWorld));
    
    const auto itemCount = pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY ? itemsInStock.size() : playerStateComponent.mPlayerBag.size();
    
    auto cursorRowIndex = 0U;
    for
    (
        auto i = itemMenuComponent.mItemMenuOffsetFromStart;
        i < math::Min(itemMenuComponent.mItemMenuOffsetFromStart + 4, static_cast<int>(itemCount));
        ++i
    )
    {
        const auto& itemNameId = pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY ? itemsInStock.at(i) : playerStateComponent.mPlayerBag.at(i).mItemName;
        const auto& itemStats  = GetItemStats(itemNameId, mWorld);
        const auto& itemPriceString = "$" + std::to_string(itemStats.mPrice);
        
        WriteTextAtTextboxCoords(itemMenuEntityId, itemNameId.GetString(), 2, 2 + cursorRowIndex * 2, mWorld);

        if (itemStats.mName != StringId("CANCEL"))
        {
            if (pokeMartDialogStateComponent.mTransactionType == TransactionType::BUY)
            {
                WriteTextAtTextboxCoords(itemMenuEntityId, itemPriceString, 14 - itemPriceString.size(), 3 + cursorRowIndex * 2, mWorld);
            }
            else
            {
                const auto& itemBagEntry = playerStateComponent.mPlayerBag[i];
                if (GetItemStats(itemBagEntry.mItemName, mWorld).mUnique == false)
                {
                    WriteTextAtTextboxCoords(itemMenuEntityId, "*", 11, 3 + cursorRowIndex * 2, mWorld);
                    WriteTextAtTextboxCoords(itemMenuEntityId, std::to_string(itemBagEntry.mQuantity), (itemBagEntry.mQuantity >= 10 ? 12 : 13), 3 + cursorRowIndex * 2, mWorld);
                }
            }
        }
        
        cursorRowIndex++;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
