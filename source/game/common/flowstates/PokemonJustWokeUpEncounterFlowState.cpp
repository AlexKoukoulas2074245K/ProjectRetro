//
//  PokemonJustWokeUpEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 20/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "DamageCalculationEncounterFlowState.h"
#include "PokemonJustWokeUpEncounterFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokemonJustWokeUpEncounterFlowState::PokemonJustWokeUpEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    
    std::string wokeUpText = "";
    if (activePlayerPokemon.mNumberOfRoundsUntilSleepEnds < 0)
    {
        activePlayerPokemon.mNumberOfRoundsUntilSleepEnds = 0;
        activePlayerPokemon.mStatus = PokemonStatus::NORMAL;
        wokeUpText += activePlayerPokemon.mName.GetString();
    }
    else if (activeOpponentPokemon.mNumberOfRoundsUntilSleepEnds < 0)
    {
        activeOpponentPokemon.mNumberOfRoundsUntilSleepEnds = 0;
        activeOpponentPokemon.mStatus = PokemonStatus::NORMAL;
        wokeUpText += "Enemy " + activeOpponentPokemon.mName.GetString();
    }
    
    wokeUpText += "#woke up!#+END";
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox(mainChatboxEntityId, wokeUpText, mWorld);
}

void PokemonJustWokeUpEncounterFlowState::VUpdate(const float)
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
