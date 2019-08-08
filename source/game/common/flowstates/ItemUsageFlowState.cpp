//
//  ItemUsageFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 08/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ItemMenuFlowState.h"
#include "ItemUsageFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float ItemUsageFlowState::ITEM_USAGE_CHATBOX_Z = -0.3f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ItemUsageFlowState::ItemUsageFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto chatboxEntityId = CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, ITEM_USAGE_CHATBOX_Z));
    QueueDialogForChatbox(chatboxEntityId, "OAK: " + playerStateComponent.mTrainerName.GetString() + "!#This isn't the#time to use that!#+END", mWorld);
}

void ItemUsageFlowState::VUpdate(const float)
{    
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE && guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        CompleteAndTransitionTo<ItemMenuFlowState>();
    }
    else if (encounterStateComponent.mActiveEncounterType == EncounterType::NONE && guiStateComponent.mActiveTextboxesStack.size() == 3)
    {
        // Destroy Use/Toss textbox
        DestroyActiveTextbox(mWorld);

        CompleteAndTransitionTo<ItemMenuFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
