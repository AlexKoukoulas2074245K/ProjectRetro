//
//  PokemonNicknameQuestionTextEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokemonNicknameQuestionTextEncounterFlowState_h
#define PokemonNicknameQuestionTextEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokemonNicknameQuestionTextEncounterFlowState final: public BaseFlowState
{
public:
    PokemonNicknameQuestionTextEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
  
private:
    void DestroyTextboxesAndTransition();
    
    static const glm::vec3 YES_NO_TEXTBOX_POSITION;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonNicknameQuestionTextEncounterFlowState_h */
