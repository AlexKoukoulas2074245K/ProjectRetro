//
//  OutOfUsablePokemonEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 04/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef OutOfUsablePokemonEncounterFlowState_h
#define OutOfUsablePokemonEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class OutOfUsablePokemonEncounterFlowState final: public BaseFlowState
{
public:
    OutOfUsablePokemonEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt);
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* OutOfUsablePokemonEncounterFlowState_h */
