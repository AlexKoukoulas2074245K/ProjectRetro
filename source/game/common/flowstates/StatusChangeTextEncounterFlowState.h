//
//  StatusChangeTextEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/08/2019.
//

#ifndef StatusChangeTextEncounterFlowState_h
#define StatusChangeTextEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class StatusChangeTextEncounterFlowState final: public BaseFlowState
{
public:
    StatusChangeTextEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


#endif /* StatusChangeTextEncounterFlowState_h */
