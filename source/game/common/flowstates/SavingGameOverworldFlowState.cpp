//
//  SavingGameOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 20/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "SavingGameOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/DirectionComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokedexStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../sound/SoundService.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/utils/OverworldUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string SavingGameOverworldFlowState::SAVE_SFX       = "general/save";
const std::string SavingGameOverworldFlowState::SAVE_FILE_NAME = "save.json";

const float SavingGameOverworldFlowState::SAVE_DELAY = 3.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

SavingGameOverworldFlowState::SavingGameOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)    
    , mSaveTimer(SAVE_DELAY)
    , mPlayerStatsTextboxEntityId(CreateSaveScreenPlayerStatsTextbox(mWorld))
{    
    SaveGame();
    QueueDialogForChatbox(CreateChatbox(mWorld), "Saving...+FREEZE", mWorld);
}

void SavingGameOverworldFlowState::VUpdate(const float dt)
{    
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() > 1 && guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        mSaveTimer.Update(dt);
        if (mSaveTimer.HasTicked())
        {
            SoundService::GetInstance().PlaySfx(SAVE_SFX, true, true);
            DestroyActiveTextbox(mWorld);
            QueueDialogForChatbox(CreateChatbox(mWorld), playerStateComponent.mPlayerTrainerName.GetString() + " saved#the game!", mWorld);
        }
    }
    else if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        // Destroy player stats
        DestroyGenericOrBareTextbox(mPlayerStatsTextboxEntityId, mWorld);

        // Destroy main menu textbox
        DestroyActiveTextbox(mWorld);

        CompleteOverworldFlow();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void SavingGameOverworldFlowState::SaveGame() const
{
    const auto& playerEntityId       = GetPlayerEntityId(mWorld);
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& movementComponent    = mWorld.GetComponent<MovementStateComponent>(playerEntityId);
    const auto& directionComponent = mWorld.GetComponent<DirectionComponent>(playerEntityId);

    std::stringstream saveFileString;
    saveFileString << "{\n";
    saveFileString << "    \"player_name\": \"" << playerStateComponent.mPlayerTrainerName.GetString() << "\",\n";
    saveFileString << "    \"rival_name\": \"" << playerStateComponent.mRivalName.GetString() << "\",\n";
    saveFileString << "    \"home_name\": \"" << playerStateComponent.mHomeLevelName.GetString() << "\",\n";
    saveFileString << "    \"current_level_name\": \"" << activeLevelComponent.mActiveLevelNameId.GetString() << "\",\n";
    saveFileString << "    \"current_game_col\": " << movementComponent.mCurrentCoords.mCol << ",\n";
    saveFileString << "    \"current_game_row\": " << movementComponent.mCurrentCoords.mRow << ",\n";
    saveFileString << "    \"current_direction\": " << static_cast<int>(directionComponent.mDirection) << ",\n";
    saveFileString << "    \"seconds_played\": " << playerStateComponent.mSecondsPlayed << ",\n";
    saveFileString << "    \"poke_dollars\": " << playerStateComponent.mPokeDollarCredits << ",\n";
    saveFileString << "    \"trainer_id\": " << playerStateComponent.mTrainerId << ",\n";
    saveFileString << "    \"badges\": [";     
    for (auto i = 0U; i < playerStateComponent.mBadgeNamesOwned.size(); ++i)
    {
        if (i != 0)
        {
            saveFileString << ", ";
        }

        saveFileString << "\"" << playerStateComponent.mBadgeNamesOwned[i].GetString() << "\"";
    }    
    saveFileString << "],\n";

    saveFileString << "    \"defeated_npc_entries\": [";
    for (auto i = 0U; i < playerStateComponent.mDefeatedNpcEntries.size(); ++i)
    {
        const auto& defeatedNpcEntry = playerStateComponent.mDefeatedNpcEntries[i];

        if (i != 0)
        {
            saveFileString << ", ";
        }

        saveFileString << "{ \"level_name\": \"" << defeatedNpcEntry.mNpcLevelName.GetString() << "\", \"level_index\": " << defeatedNpcEntry.mNpcLevelIndex << " }";
    }
    saveFileString << "],\n";

    saveFileString << "    \"collected_npc_item_entries\": [";
    for (auto i = 0U; i < playerStateComponent.mCollectedNpcItemEntries.size(); ++i)
    {
        const auto& collectedNpcItemEntry = playerStateComponent.mCollectedNpcItemEntries[i];

        if (i != 0)
        {
            saveFileString << ", ";
        }

        saveFileString << "{ \"level_name\": \"" << collectedNpcItemEntry.mNpcLevelName.GetString() << "\", \"level_index\": " << collectedNpcItemEntry.mNpcLevelIndex << " }";
    }
    saveFileString << "],\n";

    saveFileString << "    \"collected_npc_item_non_destructible_entries\": [";
    for (auto i = 0U; i < playerStateComponent.mCollectedItemNonDestructibleNpcEntries.size(); ++i)
    {
        const auto& collectedNonDestructibleNpcEntry = playerStateComponent.mCollectedItemNonDestructibleNpcEntries[i];

        if (i != 0)
        {
            saveFileString << ", ";
        }

        saveFileString << "{ \"level_name\": \"" << collectedNonDestructibleNpcEntry.mNpcLevelName.GetString() << "\", \"level_index\": " << collectedNonDestructibleNpcEntry.mNpcLevelIndex << " }";
    }
    saveFileString << "]\n";

    saveFileString << "    \"bag\": [";
    for (auto i = 0U; i < playerStateComponent.mPlayerBag.size(); ++i)
    {
        const auto& bagItemEntry = playerStateComponent.mPlayerBag[i];

        if (i != 0)
        {
            saveFileString << ", ";
        }

        saveFileString << "{ \"item_name\": \"" << bagItemEntry.mItemName.GetString() << "\", \"item_quantity\": " << bagItemEntry.mQuantity << " }";
    }
    saveFileString << "]\n";

    saveFileString << "    \"pokemon\": [";
    for (auto i = 0U; i < playerStateComponent.mPlayerPokemonRoster.size(); ++i)
    {
        const auto& pokemon = *playerStateComponent.mPlayerPokemonRoster[i];

        if (i != 0)
        {
            saveFileString << ", ";
        }

        saveFileString << "{ \"species_name\": \"" << pokemon.mBaseSpeciesStats.mSpeciesName.GetString << "\",";
        \"item_quantity\": " << bagItemEntry.mQuantity 
        saveFileString << " }";
    }
    saveFileString << "]\n";

    saveFileString << "}";

    ResourceLoadingService::GetInstance().WriteStringToFile(saveFileString.str(), ResourceLoadingService::RES_DATA_ROOT + SAVE_FILE_NAME);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

