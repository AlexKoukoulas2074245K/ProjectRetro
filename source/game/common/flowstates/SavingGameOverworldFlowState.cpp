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
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokedexStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string SavingGameOverworldFlowState::SAVE_SFX = "general/save";

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
    std::stringstream saveFileString;
    saveFileString << "{";

    saveFileString << "}";
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

