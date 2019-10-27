//
//  OaksLabPokeBallDialogOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 23/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef OaksLabPokeBallDialogOverworldFlowState_h
#define OaksLabPokeBallDialogOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseOverworldFlowState.h"
#include "../GameConstants.h"
#include "../utils/MathUtils.h"
#include "../utils/Timer.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class OaksLabPokeBallDialogOverworldFlowState final: public BaseOverworldFlowState
{
public:
    OaksLabPokeBallDialogOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
   
private:
    enum class EventState
    {
        EXCLAMATION_MARK,
        RIVAL_MOVING_TOWARD_PLAYER,        
        PLAYER_MOVING_TOWARD_WALL,
        RIVAL_SNATCHING_POKEMON_CONVERSATION,
        OAK_POST_SNATCH_CONVERSATION,
        PLAYER_MOVING_TOWARD_OAK,
        PIKACHU_RECEPTION_CONVERSATION,
        PIKACHU_NICKNAME_FLOW,
        PIKACHU_NICKNAME_YES_NO
    };

    void UpdateExclamationMark(const float dt);
    void UpdateRivalMovingTowardPlayer();
    void UpdatePlayerMovingTowardWall();
    void UpdateRivalSnatchingPokemonConversation();
    void UpdateOakPostSnatchConversation();
    void UpdatePlayerMovingTowardOak();
    void UpdatePikachuReceptionConversation();
    void UpdatePikachuNicknameFlow();
    void UpdatePikachuNicknameYesNoFlow();
    void CreateExlamationMark();
    void CreateRivalMovingToPlayerPath();
    void CreatePlayerMovingToWallPath();
    void CreatePlayerMovingToOakPath();

    static const glm::vec3 YES_NO_TEXTBOX_POSITION;
    
    static const TileCoords EXCLAMATION_MARK_ATLAS_COORDS;
    static const TileCoords OAKS_LAB_POKEBALL_COORDS;

    static const int OAKS_LAB_OAK_LEVEL_INDEX;
    static const int OAKS_LAB_RIVAL_LEVEL_INDEX;    
    static const int OAKS_LAB_POKEBALL_ENTITY_LEVEL_INDEX;
    static const float EXCLAMATION_MARK_LIFE_TIME;

    ecs::EntityId mExclamationMarkEntityId;
    Timer mTimer;
    EventState mEventState;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* OaksLabPokeBallDialogOverworldFlowState_h */
