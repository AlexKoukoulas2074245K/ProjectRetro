//
//  BallUsageResultTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 09/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BallUsageResultTextEncounterFlowState.h"
#include "TurnOverEncounterFlowState.h"
#include "PokemonNicknameQuestionTextEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

BallUsageResultTextEncounterFlowState::BallUsageResultTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    DisplayCatchResultText();
}

void BallUsageResultTextEncounterFlowState::VUpdate(const float)
{          
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        if (encounterStateComponent.mWasPokemonCaught)
        {
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, mWorld);
            mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId);
            mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId);
            
            encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId      = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId   = ecs::NULL_ENTITY_ID;
            
            CompleteAndTransitionTo<PokemonNicknameQuestionTextEncounterFlowState>();
        }
        else
        {
            encounterStateComponent.mPlayerChangedPokemonFromMainMenu = false;
            encounterStateComponent.mIsOpponentsTurn = false;
            encounterStateComponent.mTurnsCompleted = 0;
            encounterStateComponent.mLastEncounterMainMenuActionSelected = MainMenuActionType::ITEM;
            encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu = 0;
            CompleteAndTransitionTo<TurnOverEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void BallUsageResultTextEncounterFlowState::DisplayCatchResultText() const
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    // Destroy ball usage chatbox
    DestroyActiveTextbox(mWorld);

    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    std::string catchResultText = "";

    if (encounterStateComponent.mWasPokemonCaught)
    {
        catchResultText += "All right!#" + encounterStateComponent.mOpponentPokemonRoster.at(0)->mName.GetString() + " was#caught!#@";
        
        // TODO: if already caught pokemon skip this line
        catchResultText += "New POK^DEX data#will be added for#" + encounterStateComponent.mOpponentPokemonRoster.at(0)->mName.GetString() + "!#+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == -1)
    {
        catchResultText += "The trainer#blocked the BALL!#@Don't be a thief!# #+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 0)
    {
        catchResultText += "You missed the#POK^MON!#+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 1)
    {
        catchResultText += "Darn! The POK^MON#broke free!#+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 2)
    {
        catchResultText += "Aww! It appeared#to be caught!";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 3)
    {
        catchResultText += "Shoot! It was so#close too!#+END";
    }

    QueueDialogForChatbox(mainChatboxEntityId, catchResultText, mWorld);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
