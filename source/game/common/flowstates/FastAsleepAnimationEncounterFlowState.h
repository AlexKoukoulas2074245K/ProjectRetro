//
//  FastAsleepAnimationEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 20/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef FastAsleepAnimationEncounterFlowState_h
#define FastAsleepAnimationEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class FastAsleepAnimationEncounterFlowState final: public BaseFlowState
{
public:
    FastAsleepAnimationEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    void LoadFastAsleepAnimationFrames() const;
    
    static const glm::vec3 FAST_ASLEEP_ANIMATION_SCALE;
    
    static const std::string BATTLE_ANIMATION_MODEL_FILE_NAME;
    static const std::string PLAYER_FAST_ASLEEP_ANIMATION_DIR;
    static const std::string ENEMY_FAST_ASLEEP_ANIMATION_DIR;
    
    static const float FAST_ASLEEP_ANIMATION_Z;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* FastAsleepAnimationEncounterFlowState_h */
