//
//  PCMainOptionsDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PCMainOptionsDialogOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/utils/InputUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PCMainOptionsDialogOverworldFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.065f, -0.4f);

const std::string PCMainOptionsDialogOverworldFlowState::PC_TURN_OFF_SFX_NAME = "general/pc_turn_off";
const std::string PCMainOptionsDialogOverworldFlowState::PC_ENTER_SFX_NAME    = "general/pc_enter";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PCMainOptionsDialogOverworldFlowState::PCMainOptionsDialogOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mPCSystemToEnter(PCSystemType::NONE)
{
    CreatePCMainOptionsTextbox(mWorld, true);
}

void PCMainOptionsDialogOverworldFlowState::VUpdate(const float)
{
    switch (mPCSystemToEnter)
    {
        case PCSystemType::NONE: UpdateMainOptionsFlow(); break;
        case PCSystemType::BILLS_PC: UpdateEnteringPokemonPCFlow(); break;
        case PCSystemType::OAKS_PC: UpdateEnteringProfOakPCFlow(); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PCMainOptionsDialogOverworldFlowState::UpdateMainOptionsFlow()
{
    const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));

    const auto menuCursorRow = cursorComponent.mCursorRow;

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        // Pokemon PC
        if (menuCursorRow == 0)
        {
            SoundService::GetInstance().PlaySfx(PC_ENTER_SFX_NAME);
            mPCSystemToEnter = PCSystemType::BILLS_PC;           

            std::string pokemonPcEnteringText = "";

            if (playerStateComponent.mBillInteractionCompleted)
            {
                pokemonPcEnteringText = "Accessed BILL's";
            }
            else
            {
                pokemonPcEnteringText = "Accessed someone's";
            }            

            pokemonPcEnteringText += "#PC.#@Accessed POK^MON#Storage System.#+END";

            QueueDialogForChatbox(CreateChatbox(mWorld), pokemonPcEnteringText, mWorld);
        }
        // Prof Oak's PC
        else if (menuCursorRow == 1)
        {
            SoundService::GetInstance().PlaySfx(PC_ENTER_SFX_NAME);
            mPCSystemToEnter = PCSystemType::OAKS_PC;

            QueueDialogForChatbox(CreateChatbox(mWorld), "Accessed PROF.#OAK's PC.#@Accessed POK^DEX#Rating System.#@Want to get your#POK^DEX rated?+FREEZE", mWorld);
        }
        // Log off
        else
        {
            SoundService::GetInstance().PlaySfx(PC_TURN_OFF_SFX_NAME);
            DestroyActiveTextbox(mWorld);
            CompleteOverworldFlow();
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        SoundService::GetInstance().PlaySfx(PC_TURN_OFF_SFX_NAME);
        DestroyActiveTextbox(mWorld);
        CompleteOverworldFlow();
    }
}

void PCMainOptionsDialogOverworldFlowState::UpdateEnteringPokemonPCFlow()
{

}

void PCMainOptionsDialogOverworldFlowState::UpdateEnteringProfOakPCFlow()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        // Destroy options menu
        DestroyActiveTextbox(mWorld);

        // Recreate options menu
        CreatePCMainOptionsTextbox(mWorld, true);

        mPCSystemToEnter = PCSystemType::NONE;
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
        {
            CreateYesNoTextbox(mWorld, YES_NO_TEXTBOX_POSITION);
        }
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 3)
    {
        const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
        const auto& cursorComponent = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));

        const auto yesNoTextboxCursorRow = cursorComponent.mCursorRow;

        if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
        {            
            // Yes Selected
            if (yesNoTextboxCursorRow == 0)
            {
                
            }
            // No Selected
            else if (yesNoTextboxCursorRow == 1)
            {
                // Destroy yes/no textbox
                DestroyActiveTextbox(mWorld);

                // Destroy chatbox
                DestroyActiveTextbox(mWorld);

                QueueDialogForChatbox(CreateChatbox(mWorld), "Closed link to#PROF.OAK's PC.", mWorld);                
            }
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            // Destroy yes/no textbox
            DestroyActiveTextbox(mWorld);

            // Destroy chatbox
            DestroyActiveTextbox(mWorld);

            QueueDialogForChatbox(CreateChatbox(mWorld), "Closed link to#PROF.OAK's PC.", mWorld);            
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////