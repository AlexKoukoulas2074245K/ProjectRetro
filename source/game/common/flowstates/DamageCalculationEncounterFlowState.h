//
//  DamageCalculationEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/07/2019.
//

#ifndef DamageCalculationEncounterFlowState_h
#define DamageCalculationEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct Pokemon;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class DamageCalculationEncounterFlowState final: public BaseFlowState
{
public:
    DamageCalculationEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    void CalculateDamageInternal
    (
        const Pokemon& attackingPokemon,
        const Pokemon& defendingPokemon
    ) const;

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


#endif /* DamageCalculationEncounterFlowState_h */
