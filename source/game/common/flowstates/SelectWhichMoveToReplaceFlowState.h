//
//  SelectWhichMoveToReplaceFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 04/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef SelectWhichMoveToReplaceFlowState_h
#define SelectWhichMoveToReplaceFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../../common/utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class SelectWhichMoveToReplaceFlowState final: public BaseFlowState
{
public:
    SelectWhichMoveToReplaceFlowState(ecs::World&);
    
    void VUpdate(const float dt);

private:
    void TransitionToAbandonLearningNewMoveQuestion();
    
    static const glm::vec3 MOVES_TEXTBOX_POSITION;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* SelectWhichMoveToReplaceFlowState_h */
