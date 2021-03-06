//
//  PokemonJustWokeUpEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 20/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokemonJustWokeUpEncounterFlowState_h
#define PokemonJustWokeUpEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokemonJustWokeUpEncounterFlowState final: public BaseFlowState
{
public:
    PokemonJustWokeUpEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonJustWokeUpEncounterFlowState_h */
