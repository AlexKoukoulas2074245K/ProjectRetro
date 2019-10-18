//
//  OaksParcelDialogOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 13/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef OaksParcelDialogOverworldFlowState_h
#define OaksParcelDialogOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseOverworldFlowState.h"
#include "../GameConstants.h"
#include "../utils/Timer.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class OaksParcelDialogOverworldFlowState final: public BaseOverworldFlowState
{
public:
    OaksParcelDialogOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    static const std::string RIVAL_MUSIC_NAME;
    static const std::string OAKS_LAB_MUSIC_NAME;
    static const TileCoords RIVAL_ENTRANCE_COORDS;
    static const TileCoords RIVAL_OAK_SPEECH_COORDS;
    static const TileCoords RIVAL_ATLAS_COORDS;
    static const TileCoords FIRST_POKEDEX_COORDS;
    static const TileCoords SECOND_POKEDEX_COORDS;
    static const int FIRST_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX;
    static const int SECOND_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX;
    static const float POKEDEX_DISAPPEARING_DELAY_IN_SECONDS;

    enum class EventState
    {
        INTRO_DIALOG,
        RIVAL_INTRO, 
        RIVAL_ENTRANCE_PATH, 
        POKEDEX_DIALOG, 
        POKEDEX_DISAPPEARING_DELAY,
        OAK_SPEECH,
        RIVAL_SPEECH,
        RIVAL_EXIT_PATH
    };
    
    void UpdateIntroDialog();
    void UpdateRivalIntro();
    void UpdateRivalPath(const bool isEnteringScene);
    void UpdatePokedexDialog();
    void UpdatePokedexDisappearingDelay(const float dt);
    void UpdateOakSpeech();
    void UpdateRivalSpeech();    

    void CreateRivalSprite();
    void CreateRivalPath(const bool isEnteringScene);    

    EventState mEventState;
    ecs::EntityId mRivalSpriteEntityId;
    Timer mPokedexDisappearingTimer;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* OaksParcelDialogOverworldFlowState_h */
