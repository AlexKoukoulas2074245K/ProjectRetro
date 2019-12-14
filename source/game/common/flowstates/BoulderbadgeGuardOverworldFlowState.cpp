//
//  BoulderbadgeGuardOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 06/11/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BoulderbadgeGuardOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/DirectionComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/components/OverworldFlowStateSingletonComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const StringId BoulderbadgeGuardOverworldFlowState::DENIED_SFX_NAME = StringId("general/denied");

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

BoulderbadgeGuardOverworldFlowState::BoulderbadgeGuardOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mHasPlayedAccessDeniedSfx(false)
{    
    if (GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID)
    {
        DestroyActiveTextbox(mWorld);
    }
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox(mainChatboxEntityId, "Only truly skilled#trainers are#allowed through.#@You don't have the#BOULDERBADGE yet!#@The rules are#rules. I can't#let you pass.", mWorld);
}

void BoulderbadgeGuardOverworldFlowState::VUpdate(const float)
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
    else
    {
        const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
        if (guiStateComponent.mActiveChatboxContentState == ChatboxContentEndState::PARAGRAPH_END)
        {
            const auto& textboxComponent = mWorld.GetComponent<TextboxComponent>(GetActiveTextboxEntityId(mWorld));
            
            if 
            (
                mHasPlayedAccessDeniedSfx == false &&
                textboxComponent.mTextContent[4][1].mCharacter == 'B' &&
                textboxComponent.mTextContent[4][2].mCharacter == 'O'
            )
            {
                if (SoundService::GetInstance().GetLastPlayedSfxName() != DENIED_SFX_NAME)
                {
                    SoundService::GetInstance().PlaySfx(DENIED_SFX_NAME, true, true);                    
                    mHasPlayedAccessDeniedSfx = true;
                }
            }            
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


