//
//  PewterMuseumEntryOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 13/12/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PewterMuseumEntryOverworldFlowState_h
#define PewterMuseumEntryOverworldFlowState_h

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

class PewterMuseumEntryOverworldFlowState final: public BaseOverworldFlowState
{
public:
    PewterMuseumEntryOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    static const std::string PURCHASE_SFX_NAME;
    static const glm::vec3 YES_NO_TEXTBOX_POSITION;
    static const int TICKET_COST;

    void DisplayMoneyTextbox();
    void PurchaseTicketFlow();

    ecs::EntityId mMoneyTextboxEntityId;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PewterMuseumEntryOverworldFlowState_h */
