//
//  OpponentTrainerPokemonSummonTextEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef OpponentTrainerPokemonSummonTextEncounterFlowState_h
#define OpponentTrainerPokemonSummonTextEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../../common/utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class OpponentTrainerPokemonSummonTextEncounterFlowState final: public BaseFlowState
{
public:
    OpponentTrainerPokemonSummonTextEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    static const glm::vec3 OPPONENT_SPRITE_TARGET_POS;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


#endif /* OpponentTrainerPokemonSummonTextEncounterFlowState_h */
