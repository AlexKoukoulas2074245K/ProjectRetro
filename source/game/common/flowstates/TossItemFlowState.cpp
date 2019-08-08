//
//  TossItemFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 08/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ItemMenuFlowState.h"
#include "TossItemFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonItemsUtils.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float TossItemFlowState::TOSS_ITEM_CHATBOX_Z = -0.3f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TossItemFlowState::TossItemFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& itemBagEntry         = playerStateComponent.mPlayerBag.at(playerStateComponent.mPreviousItemMenuItemOffset + playerStateComponent.mPreviousItemMenuCursorRow);

    const auto chatboxEntityId = CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, TOSS_ITEM_CHATBOX_Z));
    QueueDialogForChatbox(chatboxEntityId, "Threw away#" + itemBagEntry.mItemName.GetString() + ".#+END", mWorld);    
}

void TossItemFlowState::VUpdate(const float)
{    
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& playerStateComponent       = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& itemBagEntry         = playerStateComponent.mPlayerBag.at(playerStateComponent.mPreviousItemMenuItemOffset + playerStateComponent.mPreviousItemMenuCursorRow);

    if (guiStateComponent.mActiveTextboxesStack.size() == 4)
    {   
        // If the player wants to remove the exact quantity of the bag item entry,
        // the main item menu cursor and offsets reset to 0, so that no out of bounds
        // cursors manifest
        if (playerStateComponent.mQuantityOfItemsToToss == itemBagEntry.mQuantity)
        {
            playerStateComponent.mPreviousItemMenuCursorRow  = 0;
            playerStateComponent.mPreviousItemMenuItemOffset = 0;
        }

        RemoveItemFromBag(itemBagEntry.mItemName, mWorld, playerStateComponent.mQuantityOfItemsToToss);

        // Destroy Item count selector
        DestroyActiveTextbox(mWorld);

        // Destroy Use/Toss Textbox
        DestroyActiveTextbox(mWorld);
        
        // Destroy Item Menu so that it can be reconstructed
        // in case an item was completely removed
        DestroyActiveTextbox(mWorld);

        CompleteAndTransitionTo<ItemMenuFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
