//
//  PokemonSnappedOutOfConfusionEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 16/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonSnappedOutOfConfusionEncounterFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "DamageCalculationEncounterFlowState.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokemonSnappedOutOfConfusionEncounterFlowState::PokemonSnappedOutOfConfusionEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    
    std::string snappedOutOfConfusionText = "";
    if (activePlayerPokemon.mNumberOfRoundsUntilConfusionEnds < 0)
    {
        activePlayerPokemon.mNumberOfRoundsUntilConfusionEnds = 0;
        activePlayerPokemon.mStatus = PokemonStatus::NORMAL;
        snappedOutOfConfusionText += activePlayerPokemon.mName.GetString();
    }
    else if (activeOpponentPokemon.mNumberOfRoundsUntilConfusionEnds < 0)
    {
        activeOpponentPokemon.mNumberOfRoundsUntilConfusionEnds = 0;
        activeOpponentPokemon.mStatus = PokemonStatus::NORMAL;
        snappedOutOfConfusionText += "Enemy " + activeOpponentPokemon.mName.GetString();
    }

    snappedOutOfConfusionText += "'s#confused no more!#+END";

    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox(mainChatboxEntityId, snappedOutOfConfusionText, mWorld);
}

void PokemonSnappedOutOfConfusionEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>(); 
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    { 
        CompleteAndTransitionTo<DamageCalculationEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
