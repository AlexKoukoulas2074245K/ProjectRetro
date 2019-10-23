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
        GARY_MOVING_TOWARD_PLAYER,        
        PLAYER_MOVING_TOWARD_WALL,
        GARY_SNATCHING_POKEMON_CONVERSATION,
        OAK_POST_SNATCH_CONVERSATION,
        PLAYER_MOVING_TOWARD_OAK,
        PIKACHU_RECEPTION_CONVERSATION
    };

    void UpdateExclamationMark(const float dt);
    void UpdateGaryMovingTowardPlayer();
    void UpdatePlayerMovingTowardWall();
    void UpdateGarySnatchingPokemonConversation();
    void UpdateOakPostSnatchConversation();
    void UpdatePlayerMovingTowardOak();
    void UpdatePikachuReceptionConversation();
    void CreateExlamationMark();
    void CreateGaryMovingToPlayerPath();
    void CreatePlayerMovingToWallPath();
    void CreatePlayerMovingToOakPath();

    static const TileCoords EXCLAMATION_MARK_ATLAS_COORDS;
    static const TileCoords OAKS_LAB_POKEBALL_COORDS;

    static const int OAKS_LAB_OAK_LEVEL_INDEX;
    static const int OAKS_LAB_GARY_LEVEL_INDEX;    
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
