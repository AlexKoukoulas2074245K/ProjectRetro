//
//  UseNextPokemonQuestionEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef UseNextPokemonQuestionEncounterFlowState_h
#define UseNextPokemonQuestionEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../../common/utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class UseNextPokemonQuestionEncounterFlowState final: public BaseFlowState
{
public:
    UseNextPokemonQuestionEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
  
private:
    void TransitionToPokemonSelectionView();
    void RenderGotAwaySafelyText();

    static const glm::vec3 YES_NO_TEXTBOX_POSITION;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* UseNextPokemonQuestionEncounterFlowState_h */
