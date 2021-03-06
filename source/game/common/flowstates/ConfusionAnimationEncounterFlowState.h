//
//  ConfusionAnimationEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 15/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef ConfusionAnimationEncounterFlowState_h
#define ConfusionAnimationEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class ConfusionAnimationEncounterFlowState final: public BaseFlowState
{
public:
    ConfusionAnimationEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    void LoadConfusionCheckAnimationFrames() const;

    static const glm::vec3 CONFUSION_CHECK_ANIMATION_SCALE;

    static const std::string BATTLE_ANIMATION_MODEL_FILE_NAME;
    static const std::string PLAYER_CONFUSION_CHECK_ANIMATION_DIR;
    static const std::string ENEMY_CONFUSION_CHECK_ANIMATION_DIR;
    static const std::string CONFUSION_CHECK_SFX_NAME;
    
    static const float CONFUSION_CHECK_ANIMATION_Z;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* ConfusionAnimationEncounterFlowState_h */
