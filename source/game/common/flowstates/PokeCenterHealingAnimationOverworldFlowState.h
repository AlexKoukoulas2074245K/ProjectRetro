//
//  PokeCenterHealingAnimationOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokeCenterHealingAnimationOverworldFlowState_h
#define PokeCenterHealingAnimationOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../../ECS.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/StringUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokeCenterHealingAnimationOverworldFlowState final: public BaseFlowState
{
public:
    PokeCenterHealingAnimationOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    enum class ComputerScreenOverlayEffect
    {
        NONE, WHITE, BLUE,
    };

    static const glm::vec3 PC_OVERLAY_POSITION;
    static const glm::vec3 FIRST_HEALING_BALL_POSITION;
    
    static const StringId JOY_BOW_ANIMATION_NAME;

    static const std::string HEALING_MACHINE_MODEL_NAME;
    static const std::string HEALING_BALL_MODEL_NAME;
    static const std::string HEALING_ANIMATION_SFX_NAME;
    static const std::string POKEBALL_HEALING_SFX_NAME;
    static const std::string POKE_CENTER_MUSIC_TRACK_NAME;
    static const std::string CHARACTER_MODEL_NAME;

    static const float HEALING_BALL_X_DISTANCE;
    static const float HEALING_BALL_Z_DISTANCE;
    
    static const int JOY_NPC_LEVEL_INDEX;
    static const int JOY_BOW_SPRITE_ATLAS_COL;
    static const int JOY_BOW_SPRITE_ATLAS_ROW;
    static const int FIRST_HEALING_MACHINE_GAME_COL;
    static const int FIRST_HEALING_MACHINE_GAME_ROW;
    static const int CHARACTER_ATLAS_COLS;
    static const int CHARACTER_ATLAS_ROWS;

    ecs::EntityId GetJoyEntityId() const;
    ecs::EntityId GetHealingMachineEntityId() const;
    void ShowComputerScreenOverlayEffect(const ComputerScreenOverlayEffect) const;
    void ShowHealingBallWithIndex(const size_t ballIndex, const bool invertedColors) const;
    void AddBowAnimationToJoy() const;
    void PreloadHealingMachineSkins() const;
    void SetCurrentStateDurationTimer() const;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokeCenterHealingAnimationOverworldFlowState_h */
