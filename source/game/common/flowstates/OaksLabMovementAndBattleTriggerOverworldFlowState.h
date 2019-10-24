//
//  OaksLabMovementAndBattleTriggerOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 24/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef OaksLabMovementAndBattleTriggerOverworldFlowState_h
#define OaksLabMovementAndBattleTriggerOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseOverworldFlowState.h"
#include "../GameConstants.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class OaksLabMovementAndBattleTriggerOverworldFlowState final: public BaseOverworldFlowState
{
public:
    OaksLabMovementAndBattleTriggerOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
   
private:
    enum class EventState
    {
        WAITING_FOR_GARY_TO_REACH_PLAYER,
        WAITING_FOR_BATTLE_TO_FINISH
    };

    static const std::string TRAINER_BATTLE_MAIN_MUSIC_TRACK_NAME;
    static const std::string RIVAL_TRAINER_MUSIC_NAME;
    static const TileCoords OAKS_LAB_MOVEMENT_AND_BATTLE_TRIGGER_1_TILE_COORDS;
    static const int OAKS_LAB_GARY_LEVEL_INDEX;
    
    void UpdateWaitForGaryToReachPlayer();
    void UpdateWaitForBattleToFinish();
    void CreateGaryPathToPlayer();

    EventState mEventState;
    bool mIsPlayerOnLeftTile;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* OaksLabMovementAndBattleTriggerOverworldFlowState_h */
