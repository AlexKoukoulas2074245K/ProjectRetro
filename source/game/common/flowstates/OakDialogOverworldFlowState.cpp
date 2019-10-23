//
//  OakDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 13/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OakDialogOverworldFlowState.h"
#include "OaksParcelDialogOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/PokedexUtils.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OakDialogOverworldFlowState::OakDialogOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{
    DestroyActiveTextbox(mWorld);

    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    if (HasMilestone(milestones::BOULDERBADGE, mWorld))
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "OAK: Good to see#you! How is your#POK^DEX coming?#Here, let me take#a look!#@" + GetPokedexCompletionRatingText(mWorld), mWorld);
    }
    else if (HasMilestone(milestones::RECEIVED_POKEDEX, mWorld))
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "POK^MON around the#world wait for#you, " + playerStateComponent.mPlayerTrainerName.GetString() + "!", mWorld);
    }
    else if (HasMilestone(milestones::RECEIVED_OAKS_PARCEL, mWorld))
    {
        CompleteAndTransitionTo<OaksParcelDialogOverworldFlowState>();
    }                       
    else if (HasMilestone(milestones::RECEIVED_PIKACHU, mWorld))
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "OAK: If a wild#POK^MON appears,#your POK^MON can#fight against it!#@Afterward, go on#to the next town.", mWorld);
    }
    else if (HasMilestone(milestones::SEEN_OAK_FIRST_TIME, mWorld))
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "OAK: Go ahead, #it's yours!", mWorld);        
    }
}

void OakDialogOverworldFlowState::VUpdate(const float)
{    
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        CompleteOverworldFlow();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
