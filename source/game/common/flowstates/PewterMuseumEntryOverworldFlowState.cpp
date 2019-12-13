//
//  PewterMuseumEntryOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 13/12/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PewterMuseumEntryOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/DirectionComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/components/OverworldFlowStateSingletonComponent.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PewterMuseumEntryOverworldFlowState::PURCHASE_SFX_NAME = "general/purchase";

const glm::vec3 PewterMuseumEntryOverworldFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.065f, -0.4f);

const int PewterMuseumEntryOverworldFlowState::TICKET_COST = 50;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PewterMuseumEntryOverworldFlowState::PewterMuseumEntryOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{   
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    if (playerStateComponent.mHasPurchasedMuseumTicket)
    {
        CompleteOverworldFlow();
    }
    else
    {
        DisplayMoneyTextbox();
        QueueDialogForChatbox(CreateChatbox(mWorld), "It's $" + std::to_string(TICKET_COST) + " for a#child's ticket.#@Would you like to#come in?+FREEZE", mWorld);
    }
}

void PewterMuseumEntryOverworldFlowState::VUpdate(const float)
{
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        // If declined paying or not enough money force player one tile south
        if (playerStateComponent.mHasPurchasedMuseumTicket == false)
        {
            const auto playerEntityId = GetPlayerEntityId(mWorld);

            auto& playerMovementComponent   = mWorld.GetComponent<MovementStateComponent>(playerEntityId);
            auto& playerDirectionComponent  = mWorld.GetComponent<DirectionComponent>(playerEntityId);
            auto& playerRenderableComponent = mWorld.GetComponent<RenderableComponent>(playerEntityId);

            playerMovementComponent.mMoving = true;
            playerDirectionComponent.mDirection = Direction::SOUTH;
            ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(playerDirectionComponent.mDirection), playerRenderableComponent);
        }

        DestroyGenericOrBareTextbox(mMoneyTextboxEntityId, mWorld);
        CompleteOverworldFlow();
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 1 && guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        CreateYesNoTextbox(mWorld, YES_NO_TEXTBOX_POSITION);
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
        const auto& cursorComponent     = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));                

        const auto yesNoTextboxCursorRow = cursorComponent.mCursorRow;

        if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);

            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);

            // Yes Selected
            if (yesNoTextboxCursorRow == 0)
            {
                PurchaseTicketFlow();
            }
            // No Selected
            else if (yesNoTextboxCursorRow == 1)
            {
                QueueDialogForChatbox(CreateChatbox(mWorld), "Come again!", mWorld);
            }
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);

            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);
            
            QueueDialogForChatbox(CreateChatbox(mWorld), "Come again!", mWorld);
        }
    }
}

void PewterMuseumEntryOverworldFlowState::DisplayMoneyTextbox()
{    
    mMoneyTextboxEntityId = CreatePokeMartMoneyTextbox(mWorld);
}

void PewterMuseumEntryOverworldFlowState::PurchaseTicketFlow()
{
    DestroyGenericOrBareTextbox(mMoneyTextboxEntityId, mWorld);

    auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    if (playerStateComponent.mPokeDollarCredits >= TICKET_COST)
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "Right, $" + std::to_string(TICKET_COST) + "!#Thank you!", mWorld);
        playerStateComponent.mPokeDollarCredits -= TICKET_COST;
        playerStateComponent.mHasPurchasedMuseumTicket = true;
        SoundService::GetInstance().PlaySfx(PURCHASE_SFX_NAME);
    }
    else
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "You don't have#enough money.#@Come again!", mWorld);
        playerStateComponent.mHasPurchasedMuseumTicket = false;
    }

    mMoneyTextboxEntityId = CreatePokeMartMoneyTextbox(mWorld);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


