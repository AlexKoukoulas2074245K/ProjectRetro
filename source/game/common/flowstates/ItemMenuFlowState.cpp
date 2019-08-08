//
//  ItemMenuFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 07/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ItemCountSelectorFlowState.h"
#include "ItemMenuFlowState.h"
#include "TooImportantToTossFlowState.h"
#include "ItemUsageFlowState.h"
#include "MainMenuEncounterFlowState.h"
#include "MainMenuOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/ItemMenuStateComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/MathUtils.h"
#include "../utils/PokemonItemsUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 ItemMenuFlowState::USE_TOSS_TEXTBOX_POSITION = glm::vec3(0.442699999f, -0.368300259f, -0.2f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ItemMenuFlowState::ItemMenuFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    playerStateComponent.mQuantityOfItemsToToss = 0;

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        CreateItemMenu
        (
            world,
            playerStateComponent.mPlayerBag.size(),
            playerStateComponent.mPreviousItemMenuCursorRow,
            playerStateComponent.mPreviousItemMenuItemOffset
        );

        DisplayItemsInMenuForCurrentOffset();
    }    
}

void ItemMenuFlowState::VUpdate(const float)
{    
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // Item Menu is Active
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        UpdateItemMenu();
    }   
    // Use/Toss Menu is Active
    else if (guiStateComponent.mActiveTextboxesStack.size() == 3)
    {
        UpdateUseTossTextbox();
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void ItemMenuFlowState::UpdateItemMenu()
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto itemMenuEntityId      = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);
    auto& itemMenuStateComponent     = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        const auto& itemBagEntry = playerStateComponent.mPlayerBag.at(itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow);
        if (itemBagEntry.mItemName == CANCEL_ITEM_NAME)
        {
            CancelItemMenu();
            return;
        }
        else
        {
            SaveItemMenuState();
            const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
            if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
            {
                TransitionToItemUsageFlow();
                return;
            }
            else
            {
                CreateUseTossTextbox(mWorld, USE_TOSS_TEXTBOX_POSITION);
                return;
            }
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        CancelItemMenu();
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
                    static_cast<int>(playerStateComponent.mPlayerBag.size()) - 1
            )
            {
                itemMenuStateComponent.mItemMenuOffsetFromStart = 
                    static_cast<int>(playerStateComponent.mPlayerBag.size()) - 1 - cursorComponent.mCursorRow;
            }

            RedrawItemMenu();
        }
    }

    SaveLastFramesCursorRow();
}

void ItemMenuFlowState::UpdateUseTossTextbox()
{
    const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        const auto& cursorComponent = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));

        // Use selected
        if (cursorComponent.mCursorRow == 0)
        {
            TransitionToItemUsageFlow();
        }
        // Toss selected
        else 
        {
            const auto& itemBagEntry = playerStateComponent.mPlayerBag.at(playerStateComponent.mPreviousItemMenuItemOffset + playerStateComponent.mPreviousItemMenuCursorRow);
            if (GetItemStats(itemBagEntry.mItemName, mWorld).mUnique)
            {
                CompleteAndTransitionTo<TooImportantToTossFlowState>();
            }
            else
            {
                CompleteAndTransitionTo<ItemCountSelectorFlowState>();
            }
        }        
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        DestroyActiveTextbox(mWorld);                
    }    
}

void ItemMenuFlowState::CancelItemMenu()
{
    SaveItemMenuState();
    DestroyActiveTextbox(mWorld);

    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
    {
        CompleteAndTransitionTo<MainMenuEncounterFlowState>();
    }
    else
    {
        CompleteAndTransitionTo<MainMenuOverworldFlowState>();
    }
}

void ItemMenuFlowState::TransitionToItemUsageFlow()
{
    SaveItemMenuState();
    CompleteAndTransitionTo<ItemUsageFlowState>();
}

void ItemMenuFlowState::RedrawItemMenu() const
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto itemMenuEntityId      = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);
    auto& itemMenuStateComponent     = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);

    const auto cursorRow  = cursorComponent.mCursorRow;
    const auto itemOffset = itemMenuStateComponent.mItemMenuOffsetFromStart;

    DestroyActiveTextbox(mWorld);
    CreateItemMenu(mWorld, playerStateComponent.mPlayerBag.size(), cursorRow, itemOffset);
    DisplayItemsInMenuForCurrentOffset();
}

void ItemMenuFlowState::DisplayItemsInMenuForCurrentOffset() const
{
    const auto& itemMenuEntityId     = GetActiveTextboxEntityId(mWorld);
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& itemMenuComponent    = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);    

    auto cursorRowIndex = 0U;
    for 
    (
        auto i = itemMenuComponent.mItemMenuOffsetFromStart;
        i < math::Min(itemMenuComponent.mItemMenuOffsetFromStart + 4, static_cast<int>(playerStateComponent.mPlayerBag.size()));
        ++i
    )
    {
        const auto& itemBagEntry = playerStateComponent.mPlayerBag[i];
        WriteTextAtTextboxCoords(itemMenuEntityId, itemBagEntry.mItemName.GetString(), 2, 2 + cursorRowIndex * 2, mWorld);

        if (GetItemStats(itemBagEntry.mItemName, mWorld).mUnique == false)
        {
            WriteTextAtTextboxCoords(itemMenuEntityId, "*", 11, 3 + cursorRowIndex * 2, mWorld);
            WriteTextAtTextboxCoords(itemMenuEntityId, std::to_string(itemBagEntry.mQuantity), (itemBagEntry.mQuantity >= 10 ? 12 : 13), 3 + cursorRowIndex * 2, mWorld);
        }
        
        cursorRowIndex++;
    }
}

void ItemMenuFlowState::SaveLastFramesCursorRow() const
{
    const auto itemMenuEntityId  = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent  = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);
    auto& itemMenuStateComponent = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);

    itemMenuStateComponent.mPreviousCursorRow = cursorComponent.mCursorRow;
}

void ItemMenuFlowState::SaveItemMenuState() const
{
    const auto itemMenuEntityId        = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent        = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);
    const auto& itemMenuStateComponent = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);

    auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    playerStateComponent.mPreviousItemMenuCursorRow  = cursorComponent.mCursorRow;
    playerStateComponent.mPreviousItemMenuItemOffset = itemMenuStateComponent.mItemMenuOffsetFromStart;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
