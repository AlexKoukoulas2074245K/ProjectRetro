//
//  PCPokemonSystemDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PCMainOptionsDialogOverworldFlowState.h"
#include "PCPokemonSystemDialogOverworldFlowState.h"
#include "PokemonStatsDisplayViewFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/ItemMenuStateComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokemonStatsDisplayViewStateSingletonComponent.h"
#include "../utils/PokedexUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/PCStateSingletonComponent.h"
#include "../../sound/SoundService.h"
#include "../utils/PokemonUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PCPokemonSystemDialogOverworldFlowState::RELEASE_YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.065f, -0.4f);

const float PCPokemonSystemDialogOverworldFlowState::OVERLAID_CHATBOX_Z        = -0.2f;
const float PCPokemonSystemDialogOverworldFlowState::SECOND_OVERLAID_CHATBOX_Z = -0.6f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PCPokemonSystemDialogOverworldFlowState::PCPokemonSystemDialogOverworldFlowState(ecs::World& world)
    : BaseFlowState(world)    
{    
    auto& pcStateComponent = mWorld.GetSingletonComponent<PCStateSingletonComponent>();

    if (pcStateComponent.mRestoreFromPokemonStatsFlow == false)
    {
        // Create background textbox
        CreateChatbox(mWorld);

        // Create pc pokemon system options
        CreatePCPokemonSystemOptionsTextbox(mWorld);

        pcStateComponent.mPokemonSystemState = PokemonSystemState::OPTIONS;
        pcStateComponent.mPokemonSystemOperationType = PokemonSystemOperationType::WITHDRAW;
    }
    // Means that we do not need to re-instantiate state and dialogs 
    // e.g. transitioned to pokemon stats and back here
    else
    {        
        pcStateComponent.mRestoreFromPokemonStatsFlow = false;

        // Create pc pokemon system options
        CreatePCPokemonSystemOptionsTextbox(mWorld, static_cast<int>(pcStateComponent.mPokemonSystemOperationType));

        // Create and rest pokemon list    
        CreateAndPopulatePokemonList(GetActivePokemonCollectionForCurrentOperation(), pcStateComponent.mLastItemOffsetFromStart, pcStateComponent.mLastCursorRow);

        // Create selected pokemon options
        CreatePCPokemonSelectedOptionsTextbox(mWorld, pcStateComponent.mPokemonSystemOperationType, 1);
    }
}

void PCPokemonSystemDialogOverworldFlowState::VUpdate(const float dt)
{
    const auto& pcStateComponent = mWorld.GetSingletonComponent<PCStateSingletonComponent>();
    switch (pcStateComponent.mPokemonSystemState)
    {
        case PokemonSystemState::OPTIONS:                         UpdatePokemonSystemOptionsDialog(); break;
        case PokemonSystemState::POKEMON_LIST:                    UpdatePokemonListDialog(dt); break;
        case PokemonSystemState::SELECTED_POKEMON_OPTIONS:        UpdatePokemonSelectedOptionsDialog(); break;
        case PokemonSystemState::RELEASE_EXPLANATION_TEXT:        UpdateReleaseExplanationText(); break;
        case PokemonSystemState::RELEASE_WAIT_FOR_POKEMON_CRY:    UpdateReleaseWaitForPokemonCry(); break;
        case PokemonSystemState::RELEASE_WAIT_FOR_GOOD_BYE_TEXT:  UpdateReleaseWaitForGoodByeText(); break;
        case PokemonSystemState::WAIT_FOR_OPTION_FAILURE_TEXT:    UpdateWaitForOptionFailureText(); break;
        case PokemonSystemState::WAIT_FOR_POKEMON_CRY:            UpdateWaitForPokemonCry(); break;
        case PokemonSystemState::WAIT_FOR_OPERATION_CONFIRMATION: UpdateWaitForOperationConfirmationFlow(); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PCPokemonSystemDialogOverworldFlowState::UpdatePokemonSystemOptionsDialog()
{
    const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();    
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
    const auto menuCursorRow         = cursorComponent.mCursorRow;
        
    auto& pcStateComponent = mWorld.GetSingletonComponent<PCStateSingletonComponent>();

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        // Withdraw
        if (menuCursorRow == 0)
        {       
            // No pokemon in box
            if (playerStateComponent.mPlayerBoxedPokemon.size() == 0)
            {
                QueueDialogForChatbox
                (
                    CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, OVERLAID_CHATBOX_Z)),
                    "What? There are#no POK^MON here!#+END",
                    mWorld
                );

                pcStateComponent.mPokemonSystemState = PokemonSystemState::WAIT_FOR_OPTION_FAILURE_TEXT;
            }
            // Pokemon available in box
            else
            {
                // No space in roster
                if (playerStateComponent.mPlayerPokemonRoster.size() == MAX_ROSTER_SIZE)
                {
                    QueueDialogForChatbox
                    (
                        CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, OVERLAID_CHATBOX_Z)),
                        "You can't take#any more POK^MON.#@Deposit POK^MON#first.#+END",
                        mWorld
                    );

                    pcStateComponent.mPokemonSystemState = PokemonSystemState::WAIT_FOR_OPTION_FAILURE_TEXT;
                }
                // Space in roster
                else
                {
                    CreateAndPopulatePokemonList(playerStateComponent.mPlayerBoxedPokemon);
                    pcStateComponent.mPokemonSystemState         = PokemonSystemState::POKEMON_LIST;
                    pcStateComponent.mPokemonSystemOperationType = PokemonSystemOperationType::WITHDRAW;
                }
            }
        }
        // Deposit
        else if (menuCursorRow == 1)
        {
            // Last pokemon in player's roster
            if (playerStateComponent.mPlayerPokemonRoster.size() == 1)
            {
                QueueDialogForChatbox
                (
                    CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, OVERLAID_CHATBOX_Z)),
                    "You can't deposit#the last POK^MON#+END",
                    mWorld
                );

                pcStateComponent.mPokemonSystemState = PokemonSystemState::WAIT_FOR_OPTION_FAILURE_TEXT;
            }
            else
            {
                CreateAndPopulatePokemonList(playerStateComponent.mPlayerPokemonRoster);
                pcStateComponent.mPokemonSystemState         = PokemonSystemState::POKEMON_LIST;
                pcStateComponent.mPokemonSystemOperationType = PokemonSystemOperationType::DEPOSIT;
            }
        }
        // Release
        else if (menuCursorRow == 2)
        {
            // No pokemon in box
            if (playerStateComponent.mPlayerBoxedPokemon.size() == 0)
            {
                QueueDialogForChatbox
                (
                    CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, OVERLAID_CHATBOX_Z)),
                    "What? There are#no POK^MON here!#+END",
                    mWorld
                );

                pcStateComponent.mPokemonSystemState = PokemonSystemState::WAIT_FOR_OPTION_FAILURE_TEXT;
            }
            // Pokemon available in box
            else
            {
                CreateAndPopulatePokemonList(playerStateComponent.mPlayerBoxedPokemon);
                pcStateComponent.mPokemonSystemState         = PokemonSystemState::POKEMON_LIST;
                pcStateComponent.mPokemonSystemOperationType = PokemonSystemOperationType::RELEASE;
            }
        }
        // See Ya!
        else
        {
            // Destroy options menu
            DestroyActiveTextbox(mWorld);

            // Destroy empty chatbox
            DestroyActiveTextbox(mWorld);

            CompleteAndTransitionTo<PCMainOptionsDialogOverworldFlowState>();
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        // Destroy options menu
        DestroyActiveTextbox(mWorld);

        // Destroy empty chatbox
        DestroyActiveTextbox(mWorld);

        CompleteAndTransitionTo<PCMainOptionsDialogOverworldFlowState>();
    }
}

void PCPokemonSystemDialogOverworldFlowState::UpdatePokemonListDialog(const float dt)
{        
    const auto& inputStateComponent     = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto itemMenuEntityId         = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent         = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);
    auto& itemMenuStateComponent        = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);
    auto& pcStateComponent              = mWorld.GetSingletonComponent<PCStateSingletonComponent>();
    const auto& activePokemonCollection = GetActivePokemonCollectionForCurrentOperation();

    auto& guiStateComponent             = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    guiStateComponent.mMoreItemsCursorTimer->Update(dt);
    if (guiStateComponent.mMoreItemsCursorTimer->HasTicked())
    {
        guiStateComponent.mMoreItemsCursorTimer->Reset();

        const auto pokemonCount = activePokemonCollection.size() + 1;

        if (itemMenuStateComponent.mItemMenuOffsetFromStart + 4 < static_cast<int>(pokemonCount))
        {
            guiStateComponent.mShouldDisplayIndicationForMoreItems = !guiStateComponent.mShouldDisplayIndicationForMoreItems;

            if (guiStateComponent.mShouldDisplayIndicationForMoreItems)
            {
                WriteCharAtTextboxCoords(GetActiveTextboxEntityId(mWorld), '|', 14, 9, mWorld);
            }
            else
            {
                DeleteCharAtTextboxCoords(GetActiveTextboxEntityId(mWorld), 14, 9, mWorld);
            }
        }
    }

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {            
        const auto rosterIndex                    = itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow;
        pcStateComponent.mLastCursorRow           = cursorComponent.mCursorRow;
        pcStateComponent.mLastItemOffsetFromStart = itemMenuStateComponent.mItemMenuOffsetFromStart;

        if (rosterIndex == static_cast<int>(activePokemonCollection.size()))
        {
            DestroyActiveTextbox(mWorld);
            pcStateComponent.mPokemonSystemState = PokemonSystemState::OPTIONS;
            return;
        }
        else
        {
            // Deposit/Withdraw flows
            if (pcStateComponent.mPokemonSystemOperationType != PokemonSystemOperationType::RELEASE)
            {
                pcStateComponent.mLastSelectedPokemonIndex = rosterIndex;
                CreatePCPokemonSelectedOptionsTextbox(mWorld, pcStateComponent.mPokemonSystemOperationType);
                pcStateComponent.mPokemonSystemState = PokemonSystemState::SELECTED_POKEMON_OPTIONS;
                return;
            }
            // Release flow
            else
            {
                pcStateComponent.mLastSelectedPokemonIndex = rosterIndex;
                QueueDialogForChatbox
                (
                    CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, SECOND_OVERLAID_CHATBOX_Z)),
                    "Once released,#" + activePokemonCollection.at(pcStateComponent.mLastSelectedPokemonIndex)->mName.GetString() + " is#gone forever. OK?+FREEZE",
                    mWorld
                );

                pcStateComponent.mPokemonSystemState = PokemonSystemState::RELEASE_EXPLANATION_TEXT;
                return;
            }
        }        
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        DestroyActiveTextbox(mWorld);
        pcStateComponent.mPokemonSystemState = PokemonSystemState::OPTIONS;
        return;
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent))
    {
        if (cursorComponent.mCursorRow == 0 && itemMenuStateComponent.mPreviousCursorRow == 0)
        {            
            if (--itemMenuStateComponent.mItemMenuOffsetFromStart <= 0)
            {
                itemMenuStateComponent.mItemMenuOffsetFromStart = 0;
            }

            RedrawPokemonList(activePokemonCollection);
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent))
    {
        if (cursorComponent.mCursorRow == 2 && itemMenuStateComponent.mPreviousCursorRow == 2)
        {     
            itemMenuStateComponent.mItemMenuOffsetFromStart++;
            if
            (
                itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow >= static_cast<int>(activePokemonCollection.size())
            )
            {
                itemMenuStateComponent.mItemMenuOffsetFromStart = static_cast<int>(activePokemonCollection.size()) - cursorComponent.mCursorRow;
            }
            
            RedrawPokemonList(activePokemonCollection);
        }
    }        

    SaveLastFramesCursorRow();
}

void PCPokemonSystemDialogOverworldFlowState::UpdatePokemonSelectedOptionsDialog()
{    
    const auto& inputStateComponent     = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& cursorComponent         = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));        
    auto& pcStateComponent              = mWorld.GetSingletonComponent<PCStateSingletonComponent>();
    
    const auto cursorRow = cursorComponent.mCursorRow;

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {   
        // Operation confirmed
        if (cursorRow == 0)
        {
            const auto& activePokemonCollection = GetActivePokemonCollectionForCurrentOperation();
            const auto& selectedPokemon = *activePokemonCollection.at(pcStateComponent.mLastSelectedPokemonIndex);

            // Play pokemon cry
            SoundService::GetInstance().PlaySfx("cries/" + GetFormattedPokemonIdString(selectedPokemon.mBaseSpeciesStats.mId));            

            pcStateComponent.mPokemonSystemState = PokemonSystemState::WAIT_FOR_POKEMON_CRY;
        }
        // Pokemon stats
        else if (cursorRow == 1)
        {
            // Destroy selected options 
            DestroyActiveTextbox(mWorld);

            // Destroy pokemon list
            DestroyActiveTextbox(mWorld);

            // Destroy main options textbox
            DestroyActiveTextbox(mWorld);

            pcStateComponent.mRestoreFromPokemonStatsFlow = true;
            mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>().mSourceCreatorFlow = PokemonStatsDisplayViewCreationSourceType::PC;
            CompleteAndTransitionTo<PokemonStatsDisplayViewFlowState>();
        }
        // Cancel
        else
        {
            // Destroy selected options 
            DestroyActiveTextbox(mWorld);
            
            // Destroy pokemon list
            DestroyActiveTextbox(mWorld);

            pcStateComponent.mPokemonSystemState = PokemonSystemState::OPTIONS;
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        // Destroy selected options 
        DestroyActiveTextbox(mWorld);

        // Destroy pokemon list
        DestroyActiveTextbox(mWorld);

        pcStateComponent.mPokemonSystemState = PokemonSystemState::OPTIONS;
    }
}

void PCPokemonSystemDialogOverworldFlowState::UpdateReleaseExplanationText()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 4)
    {
        if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
        {
            CreateYesNoTextbox(mWorld, RELEASE_YES_NO_TEXTBOX_POSITION);
        }
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 5)
    {
        const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
        const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));                
        const auto yesNoTextboxCursorRow = cursorComponent.mCursorRow;

        auto& pcStateComponent = mWorld.GetSingletonComponent<PCStateSingletonComponent>();

        if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);

            // Yes Selected
            if (yesNoTextboxCursorRow == 0)
            {
                const auto& activePokemonCollection = GetActivePokemonCollectionForCurrentOperation();
                const auto& selectedPokemon = *activePokemonCollection.at(pcStateComponent.mLastSelectedPokemonIndex);

                // Play pokemon cry
                SoundService::GetInstance().PlaySfx("cries/" + GetFormattedPokemonIdString(selectedPokemon.mBaseSpeciesStats.mId));

                pcStateComponent.mPokemonSystemState = PokemonSystemState::RELEASE_WAIT_FOR_POKEMON_CRY;
            }
            // No Selected
            else if (yesNoTextboxCursorRow == 1)
            {
                // Destroy main chatbox
                DestroyActiveTextbox(mWorld);

                pcStateComponent.mPokemonSystemState = PokemonSystemState::POKEMON_LIST;
            }
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);

            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);

            pcStateComponent.mPokemonSystemState = PokemonSystemState::POKEMON_LIST;
        }
    }
}

void PCPokemonSystemDialogOverworldFlowState::UpdateReleaseWaitForPokemonCry()
{
    if (SoundService::GetInstance().IsPlayingSfx()) return;

    auto& pcStateComponent = mWorld.GetSingletonComponent<PCStateSingletonComponent>();

    const auto& activePokemonCollection = GetActivePokemonCollectionForCurrentOperation();
    const auto& selectedPokemon         = *activePokemonCollection.at(pcStateComponent.mLastSelectedPokemonIndex);

    // Destroy main chatbox
    DestroyActiveTextbox(mWorld);

    // Create operation confirmation chatbox
    QueueDialogForChatbox
    (
        CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, SECOND_OVERLAID_CHATBOX_Z)),
        selectedPokemon.mName.GetString() + " was#released outside.#Bye " + selectedPokemon.mName.GetString() + "!#+END",
        mWorld
    );
    
    pcStateComponent.mPokemonSystemState = PokemonSystemState::RELEASE_WAIT_FOR_GOOD_BYE_TEXT;
}

void PCPokemonSystemDialogOverworldFlowState::UpdateReleaseWaitForGoodByeText()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& pcStateComponent     = mWorld.GetSingletonComponent<PCStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 3)
    {
        playerStateComponent.mPlayerBoxedPokemon.erase(playerStateComponent.mPlayerBoxedPokemon.begin() + pcStateComponent.mLastSelectedPokemonIndex);        

        // Destroy pokemon selection list
        DestroyActiveTextbox(mWorld);

        pcStateComponent.mPokemonSystemState = PokemonSystemState::OPTIONS;
    }
}

void PCPokemonSystemDialogOverworldFlowState::UpdateWaitForOptionFailureText()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        auto& pcStateComponent = mWorld.GetSingletonComponent<PCStateSingletonComponent>();
        pcStateComponent.mPokemonSystemState = PokemonSystemState::OPTIONS;
    }
}

void PCPokemonSystemDialogOverworldFlowState::UpdateWaitForPokemonCry()
{
    if (SoundService::GetInstance().IsPlayingSfx()) return;

    auto& pcStateComponent = mWorld.GetSingletonComponent<PCStateSingletonComponent>();

    const auto& activePokemonCollection = GetActivePokemonCollectionForCurrentOperation();
    const auto& selectedPokemon         = *activePokemonCollection.at(pcStateComponent.mLastSelectedPokemonIndex);

    std::string operationConfirmationText = "";

    switch (pcStateComponent.mPokemonSystemOperationType)
    {
        case PokemonSystemOperationType::WITHDRAW:
        {
            operationConfirmationText = selectedPokemon.mName.GetString() + " is#taken out.#Got " + selectedPokemon.mName.GetString() + ".#+END";
        } break;

        case PokemonSystemOperationType::DEPOSIT: 
        {
            operationConfirmationText = selectedPokemon.mName.GetString() + " was#stored in Box.#+END";
        } break;

        case PokemonSystemOperationType::RELEASE:
        {
        }break;
    }
    // Create operation confirmation chatbox
    QueueDialogForChatbox
    (
        CreateChatbox(mWorld, glm::vec3(CHATBOX_POSITION.x, CHATBOX_POSITION.y, SECOND_OVERLAID_CHATBOX_Z)),
        operationConfirmationText,
        mWorld
    );

    pcStateComponent.mPokemonSystemState = PokemonSystemState::WAIT_FOR_OPERATION_CONFIRMATION;
}

void PCPokemonSystemDialogOverworldFlowState::UpdateWaitForOperationConfirmationFlow()
{
    auto& pcStateComponent     = mWorld.GetSingletonComponent<PCStateSingletonComponent>();
    auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // Operation confirmation chatbox was destroyed
    if (guiStateComponent.mActiveTextboxesStack.size() == 4)
    {
        // Destroy selected pokemon options
        DestroyActiveTextbox(mWorld);

        // Destroy pokemon list
        DestroyActiveTextbox(mWorld);

        switch (pcStateComponent.mPokemonSystemOperationType)
        {
            case PokemonSystemOperationType::WITHDRAW:
            {
                playerStateComponent.mPlayerPokemonRoster.emplace_back(std::move(playerStateComponent.mPlayerBoxedPokemon[pcStateComponent.mLastSelectedPokemonIndex]));
                playerStateComponent.mPlayerBoxedPokemon.erase(playerStateComponent.mPlayerBoxedPokemon.begin() + pcStateComponent.mLastSelectedPokemonIndex);
            } break;

            case PokemonSystemOperationType::DEPOSIT: 
            {
                playerStateComponent.mPlayerBoxedPokemon.emplace_back(std::move(playerStateComponent.mPlayerPokemonRoster[pcStateComponent.mLastSelectedPokemonIndex]));
                playerStateComponent.mPlayerPokemonRoster.erase(playerStateComponent.mPlayerPokemonRoster.begin() + pcStateComponent.mLastSelectedPokemonIndex);
            } break;

            case PokemonSystemOperationType::RELEASE:
            {
            }break;
        }

        pcStateComponent.mPokemonSystemState = PokemonSystemState::OPTIONS;
    }
}

void PCPokemonSystemDialogOverworldFlowState::CreateAndPopulatePokemonList(const std::vector<std::unique_ptr<Pokemon>>& sourcePokemonCollection, const int itemMenuOffsetFromStart /* 0 */, const int previousCursorRow /* 0 */)
{
    // + 1 for the cancel button
    const auto textboxCollectionCount = sourcePokemonCollection.size() + 1;
    const auto pokemonListTextboxEntityId = CreatePCPokemonSystemPokemonListTextbox(mWorld, textboxCollectionCount, previousCursorRow, itemMenuOffsetFromStart);

    auto& itemMenuComponent = mWorld.GetComponent<ItemMenuStateComponent>(pokemonListTextboxEntityId);
    itemMenuComponent.mItemMenuOffsetFromStart = itemMenuOffsetFromStart;
    itemMenuComponent.mPreviousCursorRow       = previousCursorRow;

    DisplayPokemonListForCurrentOffset(sourcePokemonCollection);
}

void PCPokemonSystemDialogOverworldFlowState::RedrawPokemonList(const std::vector<std::unique_ptr<Pokemon>>& sourcePokemonCollection)
{
    // + 1 for the cancel button
    const auto textboxCollectionCount = sourcePokemonCollection.size() + 1;

    const auto itemMenuEntityId  = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent  = mWorld.GetComponent<CursorComponent>(itemMenuEntityId);
    auto& itemMenuStateComponent = mWorld.GetComponent<ItemMenuStateComponent>(itemMenuEntityId);

    const auto cursorRow = cursorComponent.mCursorRow;
    const auto itemOffset = itemMenuStateComponent.mItemMenuOffsetFromStart;

    DestroyActiveTextbox(mWorld);
    CreatePCPokemonSystemPokemonListTextbox(mWorld, textboxCollectionCount, cursorRow, itemOffset);
    DisplayPokemonListForCurrentOffset(sourcePokemonCollection);    
}

void PCPokemonSystemDialogOverworldFlowState::DisplayPokemonListForCurrentOffset(const std::vector<std::unique_ptr<Pokemon>>& sourcePokemonCollection)
{
    const auto& pokemonListTextboxEntityId = GetActiveTextboxEntityId(mWorld);
    const auto& pokemonListMenuComponent = mWorld.GetComponent<ItemMenuStateComponent>(pokemonListTextboxEntityId);

    // + 1 for the cancel button
    const auto textboxCollectionCount = sourcePokemonCollection.size() + 1;

    auto cursorRowIndex = 0U;
    for
    (
        auto i = pokemonListMenuComponent.mItemMenuOffsetFromStart;
        i < math::Min(pokemonListMenuComponent.mItemMenuOffsetFromStart + 4, static_cast<int>(textboxCollectionCount));
        ++i
    )
    {

        if (i == static_cast<int>(textboxCollectionCount - 1))
        {
            WriteTextAtTextboxCoords(pokemonListTextboxEntityId, "CANCEL", 2, 2 + cursorRowIndex * 2, mWorld);
        }
        else
        {
            const auto& pokemon = *sourcePokemonCollection.at(i);
            const auto levelString = "=" + std::to_string(pokemon.mLevel);

            WriteTextAtTextboxCoords(pokemonListTextboxEntityId, pokemon.mName.GetString(), 2, 2 + cursorRowIndex * 2, mWorld);
            WriteTextAtTextboxCoords(pokemonListTextboxEntityId, levelString, 13 - levelString.size(), 3 + cursorRowIndex * 2, mWorld);
        }

        cursorRowIndex++;
    }
}

void PCPokemonSystemDialogOverworldFlowState::SaveLastFramesCursorRow()
{
    const auto activeTextboxEntityId = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(activeTextboxEntityId);
    auto& itemMenuStateComponent     = mWorld.GetComponent<ItemMenuStateComponent>(activeTextboxEntityId);

    itemMenuStateComponent.mPreviousCursorRow = cursorComponent.mCursorRow;
}

const std::vector<std::unique_ptr<Pokemon>>& PCPokemonSystemDialogOverworldFlowState::GetActivePokemonCollectionForCurrentOperation() const
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& pcStateComponent     = mWorld.GetSingletonComponent<PCStateSingletonComponent>();
    return pcStateComponent.mPokemonSystemOperationType == PokemonSystemOperationType::DEPOSIT ? playerStateComponent.mPlayerPokemonRoster : playerStateComponent.mPlayerBoxedPokemon;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////