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
#include "../components/ItemMenuStateComponent.h"
#include "../components/MarketStocksSingletonComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/PokemonItemsUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/TownMapLocationDataSingletonComponent.h"
#include "../../overworld/utils/OverworldUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PokeMartBuyDialogOverworldFlowState::TEXTBOX_CLICK_SFX_NAME = "general/textbox_click";
const float PokeMartBuyDialogOverworldFlowState::TAKE_YOUR_TIME_CHATBOX_Z = -0.1f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokeMartBuyDialogOverworldFlowState::PokeMartBuyDialogOverworldFlowState(ecs::World& world)
: BaseFlowState(world)
{
    QueueDialogForChatbox(CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, TAKE_YOUR_TIME_CHATBOX_Z)), "Take your time.+FREEZE", mWorld);
    
    auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::NORMAL;
}

void PokeMartBuyDialogOverworldFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    // Item menu not yet created
    if
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
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

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
        WriteTextAtTextboxCoords(itemMenuEntityId, itemPriceString, 14 - itemPriceString.size(), 3 + cursorRowIndex * 2, mWorld);
        cursorRowIndex++;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
