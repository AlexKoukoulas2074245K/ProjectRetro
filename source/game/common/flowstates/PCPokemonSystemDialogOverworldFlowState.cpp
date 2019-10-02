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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float PCPokemonSystemDialogOverworldFlowState::OVERLAID_CHATBOX_Z = -0.2f;

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
        CreatePCPokemonSelectedOptionsTextbox(mWorld, 1);
    }
}

void PCPokemonSystemDialogOverworldFlowState::VUpdate(const float dt)
{
    const auto& pcStateComponent = mWorld.GetSingletonComponent<PCStateSingletonComponent>();
    switch (pcStateComponent.mPokemonSystemState)
    {
        case PokemonSystemState::OPTIONS:                  UpdatePokemonSystemOptionsDialog(dt); break;
        case PokemonSystemState::POKEMON_LIST:             UpdatePokemonListDialog(dt); break;
        case PokemonSystemState::SELECTED_POKEMON_OPTIONS: UpdatePokemonSelectedOptionsDialog(dt); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PCPokemonSystemDialogOverworldFlowState::UpdatePokemonSystemOptionsDialog(const float)
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
            }
            else
            {

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
            }
            // Pokemon available in box
            else
            {
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
            pcStateComponent.mLastSelectedPokemonIndex = rosterIndex;
            CreatePCPokemonSelectedOptionsTextbox(mWorld);
            pcStateComponent.mPokemonSystemState = PokemonSystemState::SELECTED_POKEMON_OPTIONS;
            return;
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

void PCPokemonSystemDialogOverworldFlowState::UpdatePokemonSelectedOptionsDialog(const float)
{
    //const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& inputStateComponent     = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& cursorComponent         = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));        
    auto& pcStateComponent              = mWorld.GetSingletonComponent<PCStateSingletonComponent>();
    //const auto& activePokemonCollection = (pcStateComponent.mPokemonSystemOperationType == PokemonSystemOperationType::DEPOSIT) ? playerStateComponent.mPlayerPokemonRoster : playerStateComponent.mPlayerBoxedPokemon;

    const auto cursorRow = cursorComponent.mCursorRow;

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {   
        // Operation confirmed
        if (cursorRow == 0)
        {
            
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

void PCPokemonSystemDialogOverworldFlowState::CreateAndPopulatePokemonList(const std::vector<std::unique_ptr<Pokemon>>& sourcePokemonCollection, const int itemMenuOffsetFromStart /* 0 */, const int previousCursorRow /* 0 */)
{
    // + 1 for the cancel button
    const auto textboxCollectionCount = sourcePokemonCollection.size() + 1;
    const auto pokemonListTextboxEntityId = CreatePCPokemonSystemPokemonListTextbox(mWorld, textboxCollectionCount);

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