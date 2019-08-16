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
#include "TurnOverEncounterFlowState.h"
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
    
    auto effectivenessFactor = 1.0f;

    if (encounterStateComponent.mDidConfusedPokemonHurtItself == false)
    {        
        effectivenessFactor = GetTypeEffectiveness(selectedMoveStats.mType, defendingPokemon.mBaseSpeciesStats.mFirstType, mWorld);
    }
         
    if (defendingPokemon.mBaseSpeciesStats.mSecondType != StringId() && encounterStateComponent.mDidConfusedPokemonHurtItself == false)
    {
        effectivenessFactor *= GetTypeEffectiveness(selectedMoveStats.mType, defendingPokemon.mBaseSpeciesStats.mSecondType, mWorld);
    }
    
    if (encounterStateComponent.mNothingHappendFromMoveExecution)
    {
        const auto mainChatboxEntityId = CreateChatbox(world);

        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            "Nothing happened!# #+END",
            mWorld
        );
    }
    else if (effectivenessFactor < 0.1f || encounterStateComponent.mMoveHadNoEffect)
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
        TestHpAndTransition();
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
        TestHpAndTransition();
    }
}

void MoveEffectivenessTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    // This is in all cases 2, except for when a critical hit is achieved
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        TestHpAndTransition();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void MoveEffectivenessTextEncounterFlowState::TestHpAndTransition()
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    auto& activePlayerPokemon = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];

    if 
    (
        activePlayerPokemon.mHp <= 0 ||
        activeOpponentPokemon.mHp <= 0
    )
    {
        CompleteAndTransitionTo<TurnOverEncounterFlowState>();
    }
    else
    {
        CompleteAndTransitionTo<MoveSideEffectTextEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
