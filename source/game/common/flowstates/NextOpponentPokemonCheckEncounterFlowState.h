//
//  NextOpponentPokemonCheckEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef NextOpponentPokemonCheckEncounterFlowState_h
#define NextOpponentPokemonCheckEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../../common/utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class NextOpponentPokemonCheckEncounterFlowState final: public BaseFlowState
{
public:
    NextOpponentPokemonCheckEncounterFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    void CreateOpponentRosterDisplay() const;
    void TransitionToPokemonSelectionView();
    void TransitionToNextOpponentPokemonState();

    static const glm::vec3 OPPONENT_ROSTER_DISPLAY_POSITION;
    static const glm::vec3 OPPONENT_ROSTER_DISPLAY_SCALE;
    static const glm::vec3 OPPONENT_INFO_TEXTBOX_POSITION;
    static const glm::vec3 YES_NO_TEXTBOX_POSITION;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* NextOpponentPokemonCheckEncounterFlowState_h */
