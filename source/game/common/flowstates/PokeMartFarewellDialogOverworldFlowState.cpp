//
//  PokeMartFarewellDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 26/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokeMartFarewellDialogOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PokeMartDialogStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float PokeMartFarewellDialogOverworldFlowState::FAREWELL_CHATBOX_Z = -0.2f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokeMartFarewellDialogOverworldFlowState::PokeMartFarewellDialogOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{
    QueueDialogForChatbox(CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, FAREWELL_CHATBOX_Z)), "Thank you!", mWorld);
    
    auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::NORMAL;
}

void PokeMartFarewellDialogOverworldFlowState::VUpdate(const float)
{
    auto& pokeMartDialogStateComponent = mWorld.GetSingletonComponent<PokeMartDialogStateSingletonComponent>();
    auto& guiStateComponent            = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        // Destroy menu textbox
        DestroyActiveTextbox(mWorld);
        
        // Destroy intro chatbox
        DestroyActiveTextbox(mWorld);
        
        // Destroy money textbox
        DestroyGenericOrBareTextbox(pokeMartDialogStateComponent.mMoneyTextboxEntityId, mWorld);
        
        pokeMartDialogStateComponent.mMenuTextboxEntityId  = ecs::NULL_ENTITY_ID;
        pokeMartDialogStateComponent.mMoneyTextboxEntityId = ecs::NULL_ENTITY_ID;
        
        CompleteOverworldFlow();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
