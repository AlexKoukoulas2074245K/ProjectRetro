//
//  TransitionAnimationSystem.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef TransitionAnimationSystem_h
#define TransitionAnimationSystem_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class TransitionAnimationSystem final: public ecs::BaseSystem
{
public:
    TransitionAnimationSystem(ecs::World&);
    
    void VUpdateAssociatedComponents(const float dt) const override;
   
private:
    static const std::string TRANSITION_ANIM_MODEL_FILE_NAME;
    static const float WARP_TRANSITION_STEP_DURATION;
    static const float WILD_FLASH_ANIMATION_STEP_DURATION;
    static const float ENCOUNTER_ANIMATION_FRAME_DURATION;
    static const int TRANSITION_STEP_COUNT;
    static const int WILD_FLASH_CYCLE_REPEAT_COUNT;

    void UpdateWarpTransitionAnimation(const float dt) const;
    void UpdateWildFlashTransitionAnimation(const float dt) const;
    void UpdateEncounterTransitionAnimation(const float dt) const;
    void LoadEncounterSpecificAnimation() const;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TransitionAnimationSystem_h */
