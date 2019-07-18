//
//  NewMovesCheckFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef NewMovesCheckFlowState_h
#define NewMovesCheckFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class NewMovesCheckFlowState final: public BaseFlowState
{
public:
    NewMovesCheckFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
   
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* AwardLevelFlowState_h */
