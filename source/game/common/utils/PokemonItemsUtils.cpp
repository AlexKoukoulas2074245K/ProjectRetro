//
//  PokemonItemsUtils.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 07/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonItemsUtils.h"
#include "../components/ItemStatsSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/ResourceLoadingService.h"

#include <json.hpp>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::string ITEMS_STATS_FILE_NAME = "items.json";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void LoadAndPopulateItemsStats(ItemStatsSingletonComponent& itemsStatsComponent)
{
    static const std::unordered_map<int, ItemUsageType> itemUsageToEnumEntry = 
    {
        { -1, ItemUsageType::UNUSABLE },
        { 0, ItemUsageType::OVERWORLD },
        { 1, ItemUsageType::BATTLE    },
        { 2, ItemUsageType::ANYWHERE  }
    };

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    // Get move stats data file resource
    const auto itemsStatsFilePath = ResourceLoadingService::RES_DATA_ROOT + ITEMS_STATS_FILE_NAME;
    resourceLoadingService.LoadResource(itemsStatsFilePath);
    const auto& itemsStatsFileResource = resourceLoadingService.GetResource<DataFileResource>(itemsStatsFilePath);

    // Parse base stats json
    const auto itemStatsJson = nlohmann::json::parse(itemsStatsFileResource.GetContents());

    for (auto it = itemStatsJson.begin(); it != itemStatsJson.end(); ++it)
    {
        const auto itemName = StringId(it.key());
        const auto& statsObject = it.value();

        const auto usage      = itemUsageToEnumEntry.at(statsObject["usage"].get<int>());
        const auto uniqueness = statsObject["unique"].get<bool>();
        const auto effect     = statsObject["effect"].get<std::string>();
        const auto price      = statsObject["price"].get<int>();

        itemsStatsComponent.mItemStats.insert(std::make_pair(itemName, ItemStats
        (
            itemName,            
            StringId(effect),
            price,
            usage,
            uniqueness
        )));
    }
}

const ItemStats& GetItemStats
(
    const StringId itemName,
    const ecs::World& world
)
{
    const auto& itemStatsComponent = world.GetSingletonComponent<ItemStatsSingletonComponent>();
    return itemStatsComponent.mItemStats.at(itemName);
}

size_t GetBagItemEntryIndex
(
    const StringId itemName,
    const ecs::World& world
)
{
    const auto& playerStateComponent = world.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    for (auto i = 0U; i < playerStateComponent.mPlayerBag.size(); ++i)
    {
        if (itemName == playerStateComponent.mPlayerBag[i].mItemName)
        {
            return i;
        }
    }

    return playerStateComponent.mPlayerBag.size();
}

void InitializePlayerBag
(
    const ecs::World& world
)
{
    auto& playerStateComponent = world.GetSingletonComponent<PlayerStateSingletonComponent>();
    playerStateComponent.mPlayerBag.push_back(BagItemEntry(CANCEL_ITEM_NAME));
}

void AddItemToBag
(
    const StringId itemName,
    const ecs::World& world,
    const int quantityToAdd /* 1 */
)
{
    auto& playerStateComponent = world.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    assert
    (
        playerStateComponent.mPlayerBag.size() >= 1 && 
        GetBagItemEntryIndex(CANCEL_ITEM_NAME, world) != playerStateComponent.mPlayerBag.size() &&
        "Player's bag does not contain Cancel item"
    );
    
    // If item already exists in bag, just increase quantity of entry
    const auto bagItemEntryIndex = GetBagItemEntryIndex(itemName, world);
    if (bagItemEntryIndex != playerStateComponent.mPlayerBag.size())
    {
        playerStateComponent.mPlayerBag[bagItemEntryIndex].mQuantity += quantityToAdd;
    }
    // If item not present in bag, create new entry for item (before CANCEL)
    else
    {
        playerStateComponent.mPlayerBag.insert(playerStateComponent.mPlayerBag.end() - 1, BagItemEntry(itemName, quantityToAdd));
    }    
}

void RemoveItemFromBag
(
    const StringId itemName,
    const ecs::World& world,
    const int quantityToRemove /* 1 */
)
{
    auto& playerStateComponent = world.GetSingletonComponent<PlayerStateSingletonComponent>();

    assert
    (
        playerStateComponent.mPlayerBag.size() >= 1 &&
        GetBagItemEntryIndex(CANCEL_ITEM_NAME, world) != playerStateComponent.mPlayerBag.size() &&
        "Player's bag does not contain Cancel item"
    );

    const auto bagItemEntryIndex = GetBagItemEntryIndex(itemName, world);

    assert
    (
        bagItemEntryIndex != playerStateComponent.mPlayerBag.size() &&
        "Player's bag does not contain item to be removed"
    );

    assert
    (
        playerStateComponent.mPlayerBag[bagItemEntryIndex].mQuantity >= quantityToRemove &&
        "Quantity to remove exceeds current one in bag"
    );

    // If we want to remove the exact quantity that there currently is in the bag, completely remove entry
    if (playerStateComponent.mPlayerBag[bagItemEntryIndex].mQuantity == quantityToRemove)
    { 
        playerStateComponent.mPlayerBag.erase(playerStateComponent.mPlayerBag.begin() + bagItemEntryIndex);
    }
    // Otherwise just reduce the quantity of the entry
    else
    {
        playerStateComponent.mPlayerBag[bagItemEntryIndex].mQuantity -= quantityToRemove;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

