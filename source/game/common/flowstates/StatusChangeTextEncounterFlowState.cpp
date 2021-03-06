//
//  StatusChangeTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "StatusChangeTextEncounterFlowState.h"
#include "PoisonTickCheckEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/PokemonMoveUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/OSMessageBox.h"
#include "../utils/TextboxUtils.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "TurnOverEncounterFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

StatusChangeTextEncounterFlowState::StatusChangeTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    
    if (encounterStateComponent.mLastMoveSelected == POISON_TICK_MOVE_NAME)
    {
        CompleteAndTransitionTo<TurnOverEncounterFlowState>();
        return;
    }

    if 
    (
        encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon == PokemonStatus::NORMAL &&
        encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon == PokemonStatus::NORMAL        
    )
    {
        CompleteAndTransitionTo<PoisonTickCheckEncounterFlowState>();
        return;
    }

    if
    (
        encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon != PokemonStatus::NORMAL && 
        activeOpponentPokemon.mStatus == encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon
    )
    {
        encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::NORMAL;
        CompleteAndTransitionTo<PoisonTickCheckEncounterFlowState>();
        return;
    }

    if
    (
        encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon != PokemonStatus::NORMAL && 
        activePlayerPokemon.mStatus == encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon
    )
    {
        encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon = PokemonStatus::NORMAL;
        CompleteAndTransitionTo<PoisonTickCheckEncounterFlowState>();
        return;
    }

    const auto statusToChangeTo =
        encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon != PokemonStatus::NORMAL ?
        encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon :
        encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon;
    
    std::string statusTextString = "";
    
    if (encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon != PokemonStatus::NORMAL)
    {
        statusTextString = activePlayerPokemon.mName.GetString();
    }
    else
    {
        statusTextString = "Enemy " + activeOpponentPokemon.mName.GetString();
    }
    
    switch (statusToChangeTo)
    {
        case PokemonStatus::PARALYZED:
        {
            statusTextString += "'s#paralyzed! It may#not attack!#+END";
        } break;
        case PokemonStatus::POISONED:
        {
            statusTextString += "#was poisoned!#+END";
        } break;
        case PokemonStatus::FROZEN:
        case PokemonStatus::CONFUSED:
        {
            statusTextString += "#became confused!#+END";
        } break;
        case PokemonStatus::ASLEEP:
        {
            statusTextString += "#fell asleep!#+END";
        } break;
        case PokemonStatus::BURNED:
        {
            ShowMessageBox(MessageBoxType::WARNING, "Status text not handled", "Status: " + GetFormattedPokemonStatus(1, statusToChangeTo) + " not handled");
        } break;
        default: break;
    }
    
    if (encounterStateComponent.mIsOpponentsTurn)
    {
        //// AAAA#was poisoned!
        // once textbox is dead change status

        //// Enemy AAAAA's#paralyzed! It may#not attack!+END
    }
    
    const auto mainChatboxEntityId = CreateChatbox(world);
    QueueDialogForChatbox(mainChatboxEntityId, statusTextString, mWorld);
}

void StatusChangeTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
      
    if (encounterStateComponent.mLastMoveSelected == POISON_TICK_MOVE_NAME)
    {
        CompleteAndTransitionTo<TurnOverEncounterFlowState>();
        return;
    }

    if
    (
        encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon == PokemonStatus::NORMAL &&
        encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon == PokemonStatus::NORMAL        
    )
    {
        CompleteAndTransitionTo<PoisonTickCheckEncounterFlowState>();
        return;
    }
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        if (encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon != PokemonStatus::NORMAL)
        {
            if (encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon != PokemonStatus::CONFUSED)
            {
                DeleteTextAtTextboxRow
                (
                    encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
                    1,
                    mWorld
                );

                WriteTextAtTextboxCoords
                (
                    encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
                    GetFormattedPokemonStatus(1, encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon),
                    4,
                    1,
                    mWorld
                );

                playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mStatus =
                    encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon;
                encounterStateComponent.mPendingStatusToBeAppliedToPlayerPokemon = PokemonStatus::NORMAL;
            }            
        }
        else
        {
            if (encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon != PokemonStatus::CONFUSED)
            {
                DeleteTextAtTextboxRow
                (
                    encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId,
                    1,
                    mWorld
                );

                WriteTextAtTextboxCoords
                (
                    encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId,
                    GetFormattedPokemonStatus(1, encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon),
                    4,
                    1,
                    mWorld
                );

                encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex]->mStatus =
                    encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon;
                encounterStateComponent.mPendingStatusToBeAppliedToOpponentPokemon = PokemonStatus::NORMAL;
            }
        }

        CompleteAndTransitionTo<PoisonTickCheckEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
