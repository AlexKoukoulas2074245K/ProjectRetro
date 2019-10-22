//
//  PlayerStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 02/07/2019.
//

#ifndef PlayerStateSingletonComponent_h
#define PlayerStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/GameConstants.h"
#include "../../common/utils/StringUtils.h"

#include <bitset>
#include <vector>
#include <memory>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct BagItemEntry
{
    BagItemEntry
    (
        const StringId itemName,
        const int initialQuantity = 1
    )
        : mItemName(itemName)
        , mQuantity(initialQuantity)
    {
    }

    StringId mItemName;
    int mQuantity;
};

struct DefeatedNpcEntry
{
    DefeatedNpcEntry
    (
        const StringId npcLevelName,
        const int npcLevelIndex
    )
        : mNpcLevelName(npcLevelName)
        , mNpcLevelIndex(npcLevelIndex)
    {
        
    }
    
    const StringId mNpcLevelName;
    const int mNpcLevelIndex;
};

struct CollectedItemNpcEntry
{
    CollectedItemNpcEntry
    (
        const StringId npcLevelName,
        const int npcLevelIndex
    )
        : mNpcLevelName(npcLevelName)
        , mNpcLevelIndex(npcLevelIndex)
    {

    }

    const StringId mNpcLevelName;
    const int mNpcLevelIndex;
};

struct CollectedItemNonDestructibleNpcEntry
{
    CollectedItemNonDestructibleNpcEntry
    (
        const StringId npcLevelName,
        const int npcLevelIndex
    )
        : mNpcLevelName(npcLevelName)
        , mNpcLevelIndex(npcLevelIndex)
    {

    }

    const StringId mNpcLevelName;
    const int mNpcLevelIndex;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

namespace milestones
{
    using Milestones = unsigned long;

    static constexpr unsigned long SEEN_OAK_FIRST_TIME     = 1UL << 0;    
    static constexpr unsigned long RECEIVED_OAKS_PARCEL    = 1UL << 1;
    static constexpr unsigned long RECEIVED_POKEDEX        = 1UL << 2;
    static constexpr unsigned long FIRST_RIVAL_BATTLE_WON  = 1UL << 3;
    static constexpr unsigned long SECOND_RIVAL_BATTLE_WON = 1UL << 4;
    static constexpr unsigned long BOULDERBADGE            = 1UL << 5;
    static constexpr unsigned long CASCADEBADGE            = 1UL << 6;
    static constexpr unsigned long THUNDERBADGE            = 1UL << 7;
    static constexpr unsigned long RAINBOWBADGE            = 1UL << 8;
    static constexpr unsigned long SOULBADGE               = 1UL << 9;
    static constexpr unsigned long MARSHBADGE              = 1UL << 10;
    static constexpr unsigned long VOLCANOBADGE            = 1UL << 11;
    static constexpr unsigned long EARTHBADGE              = 1UL << 12;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PlayerStateSingletonComponent final: public ecs::IComponent
{
public:
    std::vector<std::unique_ptr<Pokemon>> mPlayerPokemonRoster;
    std::vector<std::unique_ptr<Pokemon>> mPlayerBoxedPokemon;
    std::vector<BagItemEntry> mPlayerBag;    
    std::vector<DefeatedNpcEntry> mDefeatedNpcEntries;
    std::vector<CollectedItemNpcEntry> mCollectedNpcItemEntries;        
    std::vector<CollectedItemNonDestructibleNpcEntry> mCollectedItemNonDestructibleNpcEntries;

    StringId mPlayerTrainerName        = StringId();
    StringId mRivalName                = StringId();
    StringId mHomeLevelName            = StringId("pallet_town");
    StringId mLastOverworldLevelName   = StringId();
    StringId mPendingItemToBeAdded     = StringId();
    
    milestones::Milestones mMilestones = 0ULL;

    int mSecondsPlayed                     = 0;
    int mLastNpcLevelIndexSpokenTo         = -1;
    int mPokeDollarCredits                 = 0;
    int mLastOverworldLevelOccupiedRow     = 0;
    int mLastOverworldLevelOccupiedCol     = 0;
    int mLastEngagedTrainerOccupiedRow     = 0;
    int mLastEngagedTrainerOccupiedCol     = 0;
    int mHomeLevelOccupiedRow              = 17;
    int mHomeLevelOccupiedCol              = 11;
    int mLeveledUpPokemonRosterIndex       = -1;
    int mTrainerId                         = 0;
    int mPreviousItemMenuItemOffset        = 0;
    int mPreviousItemMenuCursorRow         = 0;
    int mPreviousMainMenuCursorRow         = 0;
    int mQuantityOfItemsToToss             = 0;
    Direction mLastOverworldDirection      = Direction::SOUTH;
    Direction mLastEngagedTrainerDirection = Direction::SOUTH;
    bool mLastBattleWon                    = false;
    bool mBillInteractionCompleted         = false;
    bool mJustDefeatedGymLeader            = false;
    bool mRivalBattleJustEnded             = false;

    ItemDiscoveryType mPendingItemToBeAddedDiscoveryType = ItemDiscoveryType::NO_ITEM;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PlayerStateSingletonComponent_h */
