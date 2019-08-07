//
//  ItemMenuFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 07/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ItemMenuFlowState.h"
#include "../components/CursorComponent.h"
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

ItemMenuFlowState::ItemMenuFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    CreateItemMenu(world, playerStateComponent.mPlayerBag.size());
    DisplayItemsInMenuForCurrentOffset();
}

void ItemMenuFlowState::VUpdate(const float)
{    
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto itemMenuEntityId      = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);

    auto& itemMenuStateComponent = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);

    if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent))
    {        
        if (cursorComponent.mCursorRow == 0 && itemMenuStateComponent.mPreviousCursorRow == 0)
        {
            itemMenuStateComponent.mItemMenuOffsetFromStart--;
            if (itemMenuStateComponent.mItemMenuOffsetFromStart <= 0)
            {
                itemMenuStateComponent.mItemMenuOffsetFromStart = 0;
            }

            const auto cursorRow  = cursorComponent.mCursorRow;
            const auto itemOffset = itemMenuStateComponent.mItemMenuOffsetFromStart;

            DestroyActiveTextbox(mWorld);
            CreateItemMenu(mWorld, playerStateComponent.mPlayerBag.size(), cursorRow, itemOffset);
            DisplayItemsInMenuForCurrentOffset();
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
                itemMenuStateComponent.mItemMenuOffsetFromStart = static_cast<int>(playerStateComponent.mPlayerBag.size()) - 1 - cursorComponent.mCursorRow;
            }

            const auto cursorRow = cursorComponent.mCursorRow;
            const auto itemOffset = itemMenuStateComponent.mItemMenuOffsetFromStart;

            DestroyActiveTextbox(mWorld);
            CreateItemMenu(mWorld, playerStateComponent.mPlayerBag.size(), cursorRow, itemOffset);
            DisplayItemsInMenuForCurrentOffset();
        }
    }

    SaveLastCursorRow();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

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

void ItemMenuFlowState::SaveLastCursorRow() const
{
    const auto itemMenuEntityId  = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent  = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);
    auto& itemMenuStateComponent = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);

    itemMenuStateComponent.mPreviousCursorRow = cursorComponent.mCursorRow;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
