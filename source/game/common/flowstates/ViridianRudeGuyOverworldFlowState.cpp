//
//  ViridianRudeGuyOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ViridianRudeGuyOverworldFlowState.h"
#include "../components/DirectionComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/components/OverworldFlowStateSingletonComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../overworld/utils/OverworldUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ViridianRudeGuyOverworldFlowState::ViridianRudeGuyOverworldFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    if (GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID)
    {
        DestroyActiveTextbox(mWorld);
    }
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox(mainChatboxEntityId, "You can't go#through here!#@This is private#property!", mWorld);
}

void ViridianRudeGuyOverworldFlowState::VUpdate(const float)
{
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        const auto playerEntityId = GetPlayerEntityId(mWorld);

        auto& playerMovementComponent   = mWorld.GetComponent<MovementStateComponent>(playerEntityId);
        auto& playerDirectionComponent  = mWorld.GetComponent<DirectionComponent>(playerEntityId);
        auto& playerRenderableComponent = mWorld.GetComponent<RenderableComponent>(playerEntityId);

        playerMovementComponent.mMoving = true;
        playerDirectionComponent.mDirection = Direction::SOUTH;
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(playerDirectionComponent.mDirection), playerRenderableComponent);

        mWorld.GetSingletonComponent<OverworldFlowStateSingletonComponent>().mFlowHasJustFinished = true;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


