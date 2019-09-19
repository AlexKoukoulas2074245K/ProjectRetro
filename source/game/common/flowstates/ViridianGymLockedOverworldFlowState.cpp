//
//  ViridianGymLockedOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ViridianGymLockedOverworldFlowState.h"
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

ViridianGymLockedOverworldFlowState::ViridianGymLockedOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{
    if (GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID)
    {
        DestroyActiveTextbox(mWorld);
    }
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox(mainChatboxEntityId, "The GYM's doors#are locked...", mWorld);
}

void ViridianGymLockedOverworldFlowState::VUpdate(const float)
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

        CompleteOverworldFlow();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


