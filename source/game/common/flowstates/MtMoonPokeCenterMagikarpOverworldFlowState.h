//
//  MtMoonPokeCenterMagikarpOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 22/12/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef MtMoonPokeCenterMagikarpOverworldFlowState_h
#define MtMoonPokeCenterMagikarpOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseOverworldFlowState.h"
#include "../utils/MathUtils.h"
#include "../../ECS.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class MtMoonPokeCenterMagikarpOverworldFlowState final: public BaseOverworldFlowState
{
public:
    MtMoonPokeCenterMagikarpOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    static const glm::vec3 YES_NO_TEXTBOX_POSITION;
    static const int MAGIKARP_COST;
    
    void DisplayMoneyTextbox();
    void PurchaseMagikarpFlow();
    
    ecs::EntityId mMoneyTextboxEntityId;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* MtMoonPokeCenterMagikarpOverworldFlowState_h */