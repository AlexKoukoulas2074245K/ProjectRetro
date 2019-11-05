//
//  GameIntroFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 05/11/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef GameIntroFlowState_h
#define GameIntroFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../components/GameIntroStateSingletonComponent.h"
#include "../GameConstants.h"
#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"
#include "../utils/Timer.h"
#include "../../ECS.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class GameIntroFlowState final: public BaseFlowState
{
public:
    GameIntroFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:    
    enum class IntroCharacter
    {
        OAK, PLAYER, RIVAL, PIKACHU
    };

    void UpdateOakFadingInState(const float dt);
    void UpdateOakWelcomeSpeechState(const float dt);
    void UpdateOakWelcomeSpeechFadeOutState(const float dt);
    void UpdatePikachuMovingInState(const float dt);
    void UpdatePikachuCryTriggerState(const float dt);
    void UpdatePokemonIntroSpeechState(const float dt);
    void UpdatePokemonIntroSpeechFadeOutState(const float dt);
    void UpdatePlayerMovingInState(const float dt);
    void UpdatePlayersNameQuestionState(const float dt);
    void UpdatePlayerMovingToTheSideState(const float dt);
    void UpdatePreDefinedPlayerNameListState(const float dt);
    void UpdatePlayerMovingBackToCenterState(const float dt);
    void UpdatePlayerNameConfirmationSpeechState(const float dt);
    void UpdatePlayerNameConfirmationSpeechFadeOutState(const float dt);
    ecs::EntityId CreateCharacterSprite(const IntroCharacter) const;    
    void CreateNameSelectionList(const bool forPlayer) const;
    std::string ExtractPredefinedNameFromList() const;
    int GetColorFlipProgressionStep() const;
    void SetColorFlipProgressionStep(const int progressionStep) const;

    static const StringId PIKACHU_CRY_SFX_NAME;

    static const TileCoords NAME_SELECTION_LIST_DIMENSIONS;

    static const glm::vec3 SPRITE_POSITION_OUTSIDE_SCREEN;
    static const glm::vec3 SPRITE_POSITION_CENTER;
    static const glm::vec3 SPRITE_POSITION_SIDE;
    static const glm::vec3 NAME_SELECTION_TEXTBOX_POSITION;
    static const glm::vec3 SPRITE_SCALE;
    
    static const float SPRITE_ANIMATION_SPEED;
    static const float COLOR_FLIP_TIMER_DELAY;

    Timer mColorFlipTimer;
    ecs::EntityId mActiveCharacterSpriteEntityId;    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* DarkenedOpponentsIntroEncounterFlowState_h */

