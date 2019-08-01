//
//  MoveEffectivenessTextEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 14/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef MoveEffectivenessTextEncounterFlowState_h
#define MoveEffectivenessTextEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct Pokemon;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class MoveEffectivenessTextEncounterFlowState final : public BaseFlowState
{
public:
    MoveEffectivenessTextEncounterFlowState(ecs::World&);

    void VUpdate(const float dt) override;

private:
    void TestHpAndTransition();
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* MoveEffectivenessTextEncounterFlowState_h */
