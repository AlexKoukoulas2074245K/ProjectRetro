//
//  OaksParcelDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 13/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OaksParcelDialogOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string OaksParcelDialogOverworldFlowState::GARY_MUSIC_NAME = "gary_trainer";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OaksParcelDialogOverworldFlowState::OaksParcelDialogOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    DestroyActiveTextbox(mWorld);
    
    QueueDialogForChatbox
    (
        CreateChatbox(mWorld),
        "OAK: Oh, " + playerStateComponent.mPlayerTrainerName.GetString() + "!# #@How is my old#" +
        "POK^MON?#@Well, it seems to#like you a lot.#@You must be#talented as a#POK^MON trainer!#@" +
        "What? You have#something for me?#@" + playerStateComponent.mPlayerTrainerName.GetString() + " delivered#OAK's PARCEL.#@" +
        "Ah! This is the#custom POK^ BALL#I ordered!#Thanks, " + playerStateComponent.mPlayerTrainerName.GetString() + "!#@" +
        "By the way, I must#ask you to do#something for me.",
        mWorld
    );
    mEventState = EventState::INTRO_DIALOG;
}

void OaksParcelDialogOverworldFlowState::VUpdate(const float)
{
    switch (mEventState)
    {
        case EventState::INTRO_DIALOG: UpdateIntroDialog(); break;
        case EventState::GARY_INTRO:   UpdateGaryIntro(); break;
        case EventState::GARY_PATH:    UpdateGaryPath(); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void OaksParcelDialogOverworldFlowState::UpdateIntroDialog()
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        SoundService::GetInstance().PlayMusic(GARY_MUSIC_NAME, false);
        mEventState = EventState::GARY_INTRO;
        
        QueueDialogForChatbox(CreateChatbox(mWorld), playerStateComponent.mRivalName.GetString() + ": Gramps!", mWorld);
    }
}

void OaksParcelDialogOverworldFlowState::UpdateGaryIntro()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        mEventState = EventState::GARY_PATH;
    }
}

void OaksParcelDialogOverworldFlowState::UpdateGaryPath()
{
    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
