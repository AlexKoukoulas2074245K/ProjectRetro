//
//  MoveEffectivenessTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 14/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MoveEffectivenessTextEncounterFlowState.h"
#include "MoveSideEffectTextEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/MathUtils.h"
#include "../utils/TextboxUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


MoveEffectivenessTextEncounterFlowState::MoveEffectivenessTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // This is in all cases 2, except for when a critical hit is achieved
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        DestroyActiveTextbox(mWorld);
    }

    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& selectedMoveStats       = GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld);
    const auto& defendingPokemon = encounterStateComponent.mIsOpponentsTurn ?
            *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]:
            *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    
    auto effectivenessFactor = GetTypeEffectiveness(selectedMoveStats.mType, defendingPokemon.mBaseSpeciesStats.mFirstType, mWorld);
    
    if (defendingPokemon.mBaseSpeciesStats.mSecondType != StringId())
    {
        effectivenessFactor *= GetTypeEffectiveness(selectedMoveStats.mType, defendingPokemon.mBaseSpeciesStats.mSecondType, mWorld);
    }
    
    if (effectivenessFactor < 0.1f)
    {
        const auto mainChatboxEntityId = CreateChatbox(world);

        if (encounterStateComponent.mIsOpponentsTurn)
        {
            QueueDialogForChatbox
            (
                mainChatboxEntityId,
                "It doesn't affect#" + defendingPokemon.mName.GetString() + "!#+END",
                mWorld
            );
        }
        else
        {
            QueueDialogForChatbox
            (
                mainChatboxEntityId,
                "It doesn't affect#Enemy " + defendingPokemon.mName.GetString() + "!#+END",
                mWorld
            );
        }        
    }
    else if (selectedMoveStats.mPower == 0)
    {
        CompleteAndTransitionTo<MoveSideEffectTextEncounterFlowState>();
    }
    else if (effectivenessFactor < 0.9f)
    {
        const auto mainChatboxEntityId = CreateChatbox(world);
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            "It's not very#effective...#+END",
            mWorld
        );
    }
    else if (effectivenessFactor > 1.1f)
    {
        const auto mainChatboxEntityId = CreateChatbox(world);
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            "It's super#effective!#+END",
            mWorld
        );
    }
    else
    {
        CompleteAndTransitionTo<MoveSideEffectTextEncounterFlowState>();
    }
}

void MoveEffectivenessTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    // This is in all cases 2, except for when a critical hit is achieved
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        CompleteAndTransitionTo<MoveSideEffectTextEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

