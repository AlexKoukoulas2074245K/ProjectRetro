//
//  MoveAnimationEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/07/2019.
//

#ifndef MoveAnimationEncounterFlowState_h
#define MoveAnimationEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class MoveAnimationEncounterFlowState final: public BaseFlowState
{
public:
    MoveAnimationEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    void LoadMoveAnimationFrames() const;
    void PrepareAllGuiSpritesForTransitionFlip() const;
    void ResetAllGuiSpritesFromTransitionFlip() const;
    void PrepareAllGuiSpritesForWhiteFlip() const;
    void UpdateSpeciallyHandledMoveAnimation(const float dt);
    void UpdateNormalFrameBasedMoveAnimation();
    void UpdateConfusionAnimation();
    void UpdateHardenAnimation();
    void UpdateBideAnimation();
    void UpdateLeerAnimation();    
    void UpdateQuickAttackAnimation(const float dt);
    void UpdateTackleAnimation();
    void UpdateTailWhipAnimation();

    static const glm::vec3 PLAYER_POKEMON_SPRITE_START_POSITION;
    static const glm::vec3 PLAYER_POKEMON_SPRITE_END_POSITION;
    static const glm::vec3 OPPONENT_POKEMON_SPRITE_START_POSITION;
    static const glm::vec3 OPPONENT_POKEMON_SPRITE_END_POSITION;
    static const glm::vec3 BATTLE_ANIMATION_SCALE;

    static const std::string BATTLE_ANIMATION_MODEL_FILE_NAME;
    static const std::string BATTLE_ANIMATION_DIR_NAME;

    static const StringId DARK_FLIP_HUD_ELEMENTS_SPECIAL_CASE_SHADER_NAME;
    static const StringId TRANSITION_FLIP_HUD_ELEMENTS_SPECIAL_CASE_SHADER_NAME;
    static const StringId DEFAULT_GUI_SHADER_NAME;

    static const float SPRITE_MOVEMENT_ANIMATION_SPEED;
    static const float BATTLE_MOVE_ANIMATION_Z;
    static const float BATTLE_MOVE_ANIMATION_ENEMY_Y_DISPLACEMENT;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


#endif /* MoveAnimationEncounterFlowState_h */
