//
//  LearnNewMoveFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef LearnNewMoveFlowState_h
#define LearnNewMoveFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class LearnNewMoveFlowState final: public BaseFlowState
{
public:
    LearnNewMoveFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    static const std::string POKEMON_LEVEL_UP_SFX_NAME;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* LearnNewMoveFlowState_h */
