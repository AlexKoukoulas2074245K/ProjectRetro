//
//  PokemonSnappedOutOfConfusionEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 16/08/2019.
//

#ifndef PokemonSnappedOutOfConfusionEncounterFlowState_h
#define PokemonSnappedOutOfConfusionEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokemonSnappedOutOfConfusionEncounterFlowState final: public BaseFlowState
{
public:
    PokemonSnappedOutOfConfusionEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
   
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonSnappedOutOfConfusionEncounterFlowState_h */
