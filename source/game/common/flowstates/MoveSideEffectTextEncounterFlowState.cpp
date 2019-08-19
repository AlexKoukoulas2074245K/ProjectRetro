//
//  MoveSideEffectTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MoveSideEffectTextEncounterFlowState.h"
#include "StatusChangeShakeEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonMoveUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MoveSideEffectTextEncounterFlowState::MoveSideEffectTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activePlayerPokemon     = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon   = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    const auto& selectedMoveStats       = GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld);

    if (selectedMoveStats.mEffect != StringId() && encounterStateComponent.mNothingHappendFromMoveExecution == false)
    {
        ConstructAndDisplayMoveSideEffectText
        (
            selectedMoveStats,
            encounterStateComponent.mIsOpponentsTurn ? activeOpponentPokemon : activePlayerPokemon,
            encounterStateComponent.mIsOpponentsTurn ? activePlayerPokemon : activeOpponentPokemon
        );
    }
}

void MoveSideEffectTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // This is in all cases 2, except for when a critical hit is achieved
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        CompleteAndTransitionTo<StatusChangeShakeEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void MoveSideEffectTextEncounterFlowState::ConstructAndDisplayMoveSideEffectText
(
    const PokemonMoveStats& selectedMoveStats,
    const Pokemon& attackingPokemon,
    const Pokemon& defendingPokemon
) const
{        
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    const auto& moveEffectString = selectedMoveStats.mEffect.GetString();    

    if 
    (
        StringEndsWith(moveEffectString, "EPAR") ||
        StringEndsWith(moveEffectString, "ECON") ||
        StringEndsWith(moveEffectString, "EPOI")
    )
    {
        return;
    }

    const auto mainChatboxEntityId = CreateChatbox(mWorld);

    std::string sideEffectText = "";
    if (moveEffectString[0] == 'E')
    {        
        if (encounterStateComponent.mIsOpponentsTurn)
        {
            sideEffectText += defendingPokemon.mName.GetString() + "'s#";
        }
        else
        {
            sideEffectText += "Enemy " + defendingPokemon.mName.GetString() + "'s#";
        }
    }
    else if (moveEffectString[0] == 'P')
    {
        if (encounterStateComponent.mIsOpponentsTurn)
        {
            sideEffectText += "Enemy " + attackingPokemon.mName.GetString() + "'s#";
        }
        else
        {
            sideEffectText += attackingPokemon.mName.GetString() + "'s#";
        }
    }

    if (moveEffectString[1] == 'A')
    {
        sideEffectText += "ATTACK";
    }
    else if (moveEffectString[1] == 'D')
    {
        sideEffectText += "DEFENSE";
    }
    else if (moveEffectString[1] == 'E')
    {
        sideEffectText += "EVASION";
    }
    else if (moveEffectString[1] == 'H')
    {
        sideEffectText += "ACCURACY";
    }
    else if (moveEffectString[1] == 'S')
    {
        sideEffectText += "SPEED";
    }
    else if (moveEffectString[1] == 'X')
    {
        sideEffectText += "SPECIAL";
    }

    if (moveEffectString[3] == '1')
    {
        sideEffectText += " ";
    }
    else
    {
        sideEffectText += "#greatly ";
    }    

    if (moveEffectString[2] == '+')
    {
        sideEffectText += "rose!#+END";
    }
    else
    {
        sideEffectText += "fell!#+END";
    }

    QueueDialogForChatbox(mainChatboxEntityId, sideEffectText, mWorld);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

