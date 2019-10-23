//
//  PikachuCatchIntroSequenceOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 22/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PikachuCatchIntroSequenceOverworldFlowState_h
#define PikachuCatchIntroSequenceOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseOverworldFlowState.h"
#include "../utils/Timer.h"
#include "../../common/GameConstants.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/StringUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PikachuCatchIntroSequenceOverworldFlowState final: public BaseOverworldFlowState
{
public:
    PikachuCatchIntroSequenceOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    enum class Character
    {
        PLAYER, OAK, GARY
    };

    enum class TimelinePoint
    {
        PALLET_TOWN_PRE_ENCOUNTER,
        PALLET_TOWN_POST_ENCOUNTER,
        OAKS_LAB_ENTRY,
        OAKS_LAB_FINAL
    };

    void UpdateExclamationMark(const float dt);
    void UpdateOakEntrance();
    void UpdateOakThatWasCloseDialog();
    void UpdateWaitForPikachuCapture();
    void UpdateOakPhewDialog();
    void UpdateOakTallGrassDialog();
    void UpdateFollowingOakToLab();
    void UpdateOakMovingInLab();
    void UpdatePlayerMovingToOakInLab();
    void UpdateGaryComplainingConversation(const float dt);
    void UpdateOakUrgingPlayerToTakeBallConversation(const float dt);
    void UpdateGaryComplainingAgainConversation(const float dt);
    void UpdateOakComplainingToGaryComplaintsConversation(const float dt);
    void CreateExlamationMark();
    void PositionOakSprite(const TimelinePoint timelinePoint);
    void CreateOakEntranceScriptedPath();   
    void CreateScriptedPathToLab();
    void CreateOakPathInLab();
    void CreatePlayerMovingToOakPathInLab();
    void ChangeCharacterDirection(const Character, const TimelinePoint, const Direction) const;
    
    enum class EventState
    {
        EXCLAMATION_MARK,
        OAK_ENTRANCE, 
        OAK_THAT_WAS_CLOSE_DIALOG, 
        WAIT_FOR_PIKACHU_CAPTURE,
        OAK_PHEW_DIALOG,
        OAK_TALL_GRASS_DIALOG,
        FOLLOWING_OAK_TO_LAB,
        OAK_MOVING_IN_LAB,
        PLAYER_MOVING_TO_OAK_IN_LAB,
        GARY_COMPLAINING_CONVERSATION,
        OAK_URGING_PLAYER_TO_TAKE_BALL_CONVERSATION,
        GARY_COMPLAINING_AGAIN_CONVERSATION,
        OAK_RESPONDING_TO_GARY_COMPLAINTS_CONVERSATION
    };
   
    static const std::string OAK_APPEARS_MUSIC_NAME;
    static const std::string FOLLOW_MUSIC_NAME;
    static const std::string WILD_BATTLE_MUSIC_NAME;
    static const std::string OAKS_LAB_MUSIC_NAME;

    static const StringId PIKACHU_NAME;
    static const StringId OAKS_LAB_LEVEL_NAME;

    static const TileCoords EXCLAMATION_MARK_ATLAS_COORDS;    
    static const TileCoords OAK_ENTRANCE_COORDS;
    static const TileCoords OAK_SPEECH_COORDS_1;
    static const TileCoords OAK_SPEECH_COORDS_2;    
    
    static const int OAKS_LAB_OAK_LEVEL_INDEX;
    static const int OAKS_LAB_GARY_LEVEL_INDEX;
    static const int PALLET_OAK_LEVEL_INDEX;
    static const int PIKACHU_LEVEL;

    static const float EXCLAMATION_MARK_LIFE_TIME;

    Timer mTimer;
    ecs::EntityId mExclamationMarkEntityId;    
    EventState mEventState;
    bool mIsPlayerOnLeftTile;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PikachuCatchIntroSequenceOverworldFlowState_h */
