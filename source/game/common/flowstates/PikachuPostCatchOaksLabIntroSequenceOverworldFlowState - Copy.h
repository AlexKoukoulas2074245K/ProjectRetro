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
        PLAYER, OAK
    };

    void UpdateExclamationMark(const float dt);
    void UpdateOakEntrance();
    void UpdateOakThatWasCloseDialog();
    void CreateExlamationMark();
    void PositionOakSprite();
    void CreateOakEntranceScriptedPath();   
    void ChangeCharacterDirection(const Character, const Direction) const;

    /*
    void UpdateWaitForRivalBattleToEnd();
    void UpdateRivalDefeatedDialog();
    void UpdateRivalExit();
    */
    enum class EventState
    {
        EXCLAMATION_MARK, OAK_ENTRANCE, OAK_THAT_WAS_CLOSE_DIALOG
    };
   
    static const std::string OAK_APPEARS_MUSIC_NAME;
    static const std::string FOLLOW_MUSIC_NAME;
    static const std::string WILD_BATTLE_MUSIC_NAME;

    static const StringId PIKACHU_NAME;

    static const TileCoords EXCLAMATION_MARK_ATLAS_COORDS;    
    static const TileCoords OAK_ENTRANCE_COORDS;
    static const TileCoords OAK_SPEECH_COORDS_1;
    static const TileCoords OAK_SPEECH_COORDS_2;    
    
    static const int OAK_LEVEL_INDEX;
    static const int PIKACHU_LEVEL;

    static const float EXCLAMATION_MARK_LIFE_TIME;

    Timer mExclamationMarkTimer;
    ecs::EntityId mExclamationMarkEntityId;    
    EventState mEventState;
    bool mIsPlayerOnLeftTile;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PikachuCatchIntroSequenceOverworldFlowState_h */
