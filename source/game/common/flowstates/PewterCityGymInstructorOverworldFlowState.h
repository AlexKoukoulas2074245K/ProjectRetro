//
//  PewterCityGymInstructorOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PewterCityGymInstructorOverworldFlowState_h
#define PewterCityGymInstructorOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseOverworldFlowState.h"
#include "../../common/utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PewterCityGymInstructorOverworldFlowState final: public BaseOverworldFlowState
{
public:
    PewterCityGymInstructorOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    static const glm::vec3 YES_NO_TEXTBOX_POSITION;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PewterCityGymInstructorOverworldFlowState_h */