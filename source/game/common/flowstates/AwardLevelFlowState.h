//
//  AwardLevelFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef AwardLevelFlowState_h
#define AwardLevelFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class AwardLevelFlowState final: public BaseFlowState
{
public:
    AwardLevelFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    void RefreshPokemonStats() const;

    static const glm::vec3 POKEMON_STATS_DISPLAY_TEXTBOX_POSITION;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* AwardLevelFlowState_h */
