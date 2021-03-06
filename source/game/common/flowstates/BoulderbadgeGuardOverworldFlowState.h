//
//  BoulderbadgeGuardOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 06/11/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef BoulderbadgeGuardOverworldFlowState_h
#define BoulderbadgeGuardOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseOverworldFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class BoulderbadgeGuardOverworldFlowState final: public BaseOverworldFlowState
{
public:
    BoulderbadgeGuardOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    static const StringId DENIED_SFX_NAME;

    bool mHasPlayedAccessDeniedSfx;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* BoulderbadgeGuardOverworldFlowState_h */
