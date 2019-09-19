//
//  ViridianSchoolBlackboardOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 19/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef ViridianSchoolBlackboardOverworldFlowState_h
#define ViridianSchoolBlackboardOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseOverworldFlowState.h"
#include "../../common/utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class ViridianSchoolBlackboardOverworldFlowState final: public BaseOverworldFlowState
{
public:
    ViridianSchoolBlackboardOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    static const glm::vec3 BLACKBOARD_TEXTBOX_POSITION;
    static const int BLACKBOARD_TEXTBOX_COLS;
    static const int BLACKBOARD_TEXTBOX_ROWS;
    
    void ShowBlackboardExplanationText
    (
        const int blackboardCursorCol,
        const int blackboardCursorRow
    ) const;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* ViridianSchoolBookOverworldFlowState_h */
