//
//  BideUnleashTextEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 17/10/2019.
//

#ifndef BideUnleashTextEncounterFlowState_h
#define BideUnleashTextEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class BideUnleashTextEncounterFlowState final: public BaseFlowState
{
public:
    BideUnleashTextEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
   
private:
    std::string GetUnleashEnergyText() const;

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* BideUnleashTextEncounterFlowState_h */