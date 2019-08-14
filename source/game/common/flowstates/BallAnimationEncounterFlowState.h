//
//  BallAnimationEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 09/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef BallAnimationEncounterFlowState_h
#define BallAnimationEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class BallAnimationEncounterFlowState final: public BaseFlowState
{
public:
    BallAnimationEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    void UpdateCatchAttemptAnimation(const float dt);
    void LoadCatchAttemptFrames() const;

    static const std::string BATTLE_ANIMATION_MODEL_FILE_NAME;
    static const std::string BATTLE_ANIMATION_DIR_NAME;

    static const glm::vec3 BATTLE_MOVE_SCALE;

    static const float BATTLE_MOVE_ANIMATION_Z;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* BallAnimationEncounterFlowState_h */