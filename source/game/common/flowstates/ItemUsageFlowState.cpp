//
//  ItemUsageFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 08/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BallUsageEncounterFlowState.h"
#include "ItemMenuFlowState.h"
#include "ItemUsageFlowState.h"
#include "PokemonSelectionViewFlowState.h"
#include "TownMapOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../utils/PokemonItemsUtils.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float ItemUsageFlowState::ITEM_USAGE_CHATBOX_Z = -0.3f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ItemUsageFlowState::ItemUsageFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    if (ItemCanBeUsed() == false)
    {
        const auto chatboxEntityId = CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, ITEM_USAGE_CHATBOX_Z));
        QueueDialogForChatbox(chatboxEntityId, "OAK: " + playerStateComponent.mPlayerTrainerName.GetString() + "!#This isn't the#time to use that!#+END", mWorld);
    }
}

void ItemUsageFlowState::VUpdate(const float)
{    
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& itemBagEntry         = playerStateComponent.mPlayerBag.at(playerStateComponent.mPreviousItemMenuItemOffset + playerStateComponent.mPreviousItemMenuCursorRow);
    const auto itemName              = itemBagEntry.mItemName;
    const auto& selectedItemStats    = GetItemStats(itemBagEntry.mItemName, mWorld);
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (ItemCanBeUsed() || encounterStateComponent.mIsPikachuCaptureFlowActive)
    {
        if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
        {
            encounterStateComponent.mIsOpponentsTurn = false;
        }        

        // Ball flow
        if (selectedItemStats.mEffect == StringId("BALL") || encounterStateComponent.mIsPikachuCaptureFlowActive)
        {
            if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
            {
                if (encounterStateComponent.mIsPikachuCaptureFlowActive == false)
                {
                    RemoveItemFromBag(itemBagEntry.mItemName, mWorld, 1);
                }

                // Destroy Item Menu
                DestroyActiveTextbox(mWorld);

                const auto mainChatboxEntityId = CreateChatbox(mWorld);
                
                if (encounterStateComponent.mIsPikachuCaptureFlowActive)
                {
                    QueueDialogForChatbox
                    (
                        mainChatboxEntityId,
                        "PROF.OAK used#POK^_BALL!+FREEZE",
                        mWorld
                    );
                }
                else
                {
                    QueueDialogForChatbox
                    (
                        mainChatboxEntityId,
                        playerStateComponent.mPlayerTrainerName.GetString() + " used#" +
                        itemName.GetString() + "!+FREEZE",
                        mWorld
                    );
                }
                
                CompleteAndTransitionTo<BallUsageEncounterFlowState>();
            }
            else if (encounterStateComponent.mActiveEncounterType == EncounterType::NONE)
            {
                // Destroy Use/Toss textbox
                DestroyActiveTextbox(mWorld);

                // Destroy Item Menu
                DestroyActiveTextbox(mWorld);

                CompleteAndTransitionTo<BallUsageEncounterFlowState>();
            }
        }
        // Potion flow
        else if (StringStartsWith(selectedItemStats.mEffect.GetString(), "POTION"))
        {
            // Destroy Use/Toss textbox
            DestroyActiveTextbox(mWorld);

            // Destroy Item Menu
            DestroyActiveTextbox(mWorld);            

            auto& pokemonSelectionViewState = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
            pokemonSelectionViewState.mCreationSourceType = PokemonSelectionViewCreationSourceType::ITEM_USAGE;

            CompleteAndTransitionTo<PokemonSelectionViewFlowState>();
        }
        // Town map flow
        else if (StringStartsWith(selectedItemStats.mEffect.GetString(), "TOWNMAP"))
        {
            // Destroy Use/Toss textbox
            DestroyActiveTextbox(mWorld);

            // Destroy Item Menu
            DestroyActiveTextbox(mWorld);

            CompleteAndTransitionTo<TownMapOverworldFlowState>();
        }
    }
    else
    {
        if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE && guiStateComponent.mActiveTextboxesStack.size() == 2)
        {
            CompleteAndTransitionTo<ItemMenuFlowState>();
        }
        else if (encounterStateComponent.mActiveEncounterType == EncounterType::NONE && guiStateComponent.mActiveTextboxesStack.size() == 3)
        {
            // Destroy Use/Toss textbox
            DestroyActiveTextbox(mWorld);

            CompleteAndTransitionTo<ItemMenuFlowState>();
        }
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

bool ItemUsageFlowState::ItemCanBeUsed() const
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& itemBagEntry            = playerStateComponent.mPlayerBag.at(playerStateComponent.mPreviousItemMenuItemOffset + playerStateComponent.mPreviousItemMenuCursorRow);
    const auto& itemStats               = GetItemStats(itemBagEntry.mItemName, mWorld);
        
    if (itemStats.mUsageType == ItemUsageType::BATTLE && encounterStateComponent.mActiveEncounterType == EncounterType::NONE)
    {
        return false;
    }
    else if (itemStats.mUsageType == ItemUsageType::OVERWORLD && encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
    {
        return false;
    }
    else if (itemStats.mUsageType == ItemUsageType::UNUSABLE)
    {
        return false;
    }    
    
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
