//
//  MtMoonPokeCenterMagikarpOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 22/12/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MtMoonPokeCenterMagikarpOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/DirectionComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/MilestoneUtils.h"
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

const glm::vec3 MtMoonPokeCenterMagikarpOverworldFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.065f, -0.4f);

const int MtMoonPokeCenterMagikarpOverworldFlowState::MAGIKARP_COST = 500;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MtMoonPokeCenterMagikarpOverworldFlowState::MtMoonPokeCenterMagikarpOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{
    DestroyActiveTextbox(mWorld);
    
    if (HasMilestone(milestones::RECEIVED_MAGIKARP, mWorld))
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "MAN: Well, I don't#give refunds!", mWorld);
        CompleteOverworldFlow();
    }
    else
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "MAN: Hello, there!#Have I got a deal#just for you!#@I'll let you have#a swell MAGIKARP#for just $" + std::to_string(MAGIKARP_COST) + "#What do you say?+FREEZE", mWorld);
    }
}

void MtMoonPokeCenterMagikarpOverworldFlowState::VUpdate(const float)
{
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        DestroyGenericOrBareTextbox(mMoneyTextboxEntityId, mWorld);
        CompleteOverworldFlow();
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 1 && guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        DisplayMoneyTextbox();
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
                PurchaseMagikarpFlow();
            }
            // No Selected
            else if (yesNoTextboxCursorRow == 1)
            {
                QueueDialogForChatbox(CreateChatbox(mWorld), "No? I'm only#doing this as a#favor to you!", mWorld);
            }
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);
            
            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);
            
            QueueDialogForChatbox(CreateChatbox(mWorld), "No? I'm only#doing this as a#favor to you!", mWorld);
        }
    }
}

void MtMoonPokeCenterMagikarpOverworldFlowState::DisplayMoneyTextbox()
{
    mMoneyTextboxEntityId = CreatePokeMartMoneyTextbox(mWorld);
}

void MtMoonPokeCenterMagikarpOverworldFlowState::PurchaseMagikarpFlow()
{
    DestroyGenericOrBareTextbox(mMoneyTextboxEntityId, mWorld);
    
    auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    if (playerStateComponent.mPokeDollarCredits >= MAGIKARP_COST)
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), playerStateComponent.mPlayerTrainerName.GetString() + " got#a_MAGIKARP!", mWorld);
        playerStateComponent.mPokeDollarCredits -= MAGIKARP_COST;
        SetMilestone(milestones::RECEIVED_MAGIKARP, mWorld);
    }
    else
    {
        QueueDialogForChatbox(CreateChatbox(mWorld), "You'll need more#money than that!", mWorld);
    }
    
    mMoneyTextboxEntityId = CreatePokeMartMoneyTextbox(mWorld);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
