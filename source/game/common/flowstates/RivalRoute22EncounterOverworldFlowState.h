//
//  RivalRoute22EncounterOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 15/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef RivalRoute22EncounterOverworldFlowState_h
#define RivalRoute22EncounterOverworldFlowState_h

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

class RivalRoute22EncounterOverworldFlowState final: public BaseOverworldFlowState
{
public:
    RivalRoute22EncounterOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    void UpdateExclamationMark(const float dt);
    void UpdateRivalEntrance();
    void UpdateWaitForRivalBattleToEnd();
    void UpdateRivalDefeatedDialog();
    void UpdateRivalExit();
    void CreateExlamationMark();
    void PositionRivalSprite();
    void CreateScriptedPath(const bool isEnteringScene);   

    enum class EventState
    {
        EXCLAMATION_MARK, RIVAL_ENTRANCE, RIVAL_WAIT_FOR_BATTLE, RIVAL_DEFEATED_DIALOG, RIVAL_EXIT
    };
   
    static const std::string RIVAL_TRAINER_MUSIC_NAME;
    static const std::string LEVEL_MUSIC_NAME;

    static const TileCoords EXCLAMATION_MARK_ATLAS_COORDS;
    static const TileCoords RIVAL_ENTRANCE_COORDS;
    static const TileCoords RIVAL_SPEECH_COORDS_1;
    static const TileCoords RIVAL_SPEECH_COORDS_2;    
    
    static const int RIVAL_1_LEVEL_INDEX;
    
    static const float EXCLAMATION_MARK_LIFE_TIME;

    Timer mExclamationMarkTimer;
    ecs::EntityId mExclamationMarkEntityId;    
    EventState mEventState;
    bool mIsPlayerOnBottomTile;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* RivalRoute22EncounterOverworldFlowState_h */
