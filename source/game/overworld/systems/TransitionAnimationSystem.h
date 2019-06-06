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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class TransitionAnimationSystem final: public ecs::BaseSystem
{
public:
    TransitionAnimationSystem(ecs::World&);
    
    void VUpdateAssociatedComponents(const float dt) const override;
   
private:
    static const float TRANSITION_STEP_DURATION;
    static const int TRANSITION_STEPS_COUNT;

    void UpdateWarpTransitionAnimation(const float dt) const;
    void UpdateBattleTransitionAnimation(const float dt) const;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TransitionAnimationSystem_h */
