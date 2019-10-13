//
//  PokeMartIntroDialogOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 26/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokeMartIntroDialogOverworldFlowState_h
#define PokeMartIntroDialogOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseOverworldFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokeMartIntroDialogOverworldFlowState final: public BaseOverworldFlowState
{
public:
    PokeMartIntroDialogOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    static const int NPC_BEHIND_COUNTER_LEVEL_INDEX;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokeMartIntroDialogOverworldFlowState_h */
