//
//  PewterMuseumReceptionistSideDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 05/12/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PewterMuseumReceptionistSideDialogOverworldFlowState.h"
#include "../components/DirectionComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/utils/AnimationUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const int PewterMuseumReceptionistSideDialogOverworldFlowState::NPC_BEHIND_COUNTER_LEVEL_INDEX = 5;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PewterMuseumReceptionistSideDialogOverworldFlowState::PewterMuseumReceptionistSideDialogOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{   
    DestroyActiveTextbox(mWorld);

    // Change direction of npc behind counter
    // We can't use the last npc level index here, as actually we are not talking to the npc behind the counter,
    // but rather three different invisible npcs arount the mart counter
    auto& npcRenderableComponent   = mWorld.GetComponent<RenderableComponent>(GetNpcEntityIdFromLevelIndex(NPC_BEHIND_COUNTER_LEVEL_INDEX, mWorld));
    auto& playerDirectionComponent = mWorld.GetComponent<DirectionComponent>(GetPlayerEntityId(mWorld));
    auto& npcAiComponent           = mWorld.GetComponent<NpcAiComponent>(GetNpcEntityIdFromLevelIndex(NPC_BEHIND_COUNTER_LEVEL_INDEX, mWorld));
    auto& playerStateComponent     = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    const auto directionFacingPlayer = static_cast<Direction>((static_cast<int>(playerDirectionComponent.mDirection) + 2) % 4);
    ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(directionFacingPlayer), npcRenderableComponent);
    npcAiComponent.mAiTimer->Reset();

    std::string dialogString = "Please go to the#other side!";

    if (playerStateComponent.mHasPurchasedMuseumTicket)
    {
        dialogString = "Take plenty of#time to look!";
    }    

    QueueDialogForChatbox(CreateChatbox(mWorld), dialogString, mWorld);
    CompleteOverworldFlow();
}

void PewterMuseumReceptionistSideDialogOverworldFlowState::VUpdate(const float)
{        
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


