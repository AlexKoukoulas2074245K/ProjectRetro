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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PlayerStateSingletonComponent final: public ecs::IComponent
{
public:
    std::vector<std::unique_ptr<Pokemon>> mPlayerPokemonRoster;
    std::vector<BagItemEntry> mPlayerBag;
    std::vector<StringId> mBadgeNamesOwned;

    StringId mLastOverworldLevelName   = StringId();
    StringId mTrainerName              = StringId();
    StringId mHomeLevelName            = StringId("pallet");

    int mPokeDollarCredits             = 0;
    int mLastOverworldLevelOccupiedRow = 0;
    int mLastOverworldLevelOccupiedCol = 0;
    int mHomeLevelOccupiedRow          = 17;
    int mHomeLevelOccupiedCol          = 11;
    int mLeveledUpPokemonRosterIndex   = -1;
    int mTrainerId                     = 0;
    int mPreviousItemMenuItemOffset    = 0;
    int mPreviousItemMenuCursorRow     = 0;
    int mPreviousMainMenuCursorRow     = 0;
    int mQuantityOfItemsToToss         = 0;
    Direction mLastOverworldDirection  = Direction::SOUTH;    
    bool mBillInteractionCompleted     = false;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PlayerStateSingletonComponent_h */
