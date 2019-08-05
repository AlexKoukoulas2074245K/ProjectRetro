//
//  EvolutionTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 05/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "EvolutionTextFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

EvolutionTextFlowState::EvolutionTextFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    const auto pokemonReadyToEvolveIndex = GetReadyToEvolvePokemonRosterIndex(playerStateComponent.mPlayerPokemonRoster);
    if (pokemonReadyToEvolveIndex != playerStateComponent.mPlayerPokemonRoster.size())
    {
        const auto& mainChatboxEntityId = CreateChatbox(mWorld);
        QueueDialogForChatbox(mainChatboxEntityId, "What? " + playerStateComponent.mPlayerPokemonRoster[pokemonReadyToEvolveIndex]->mName.GetString() + "#is evolving!#+FREEZE", mWorld);
    }
}

void EvolutionTextFlowState::VUpdate(const float)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    if (GetReadyToEvolvePokemonRosterIndex(playerStateComponent.mPlayerPokemonRoster) == playerStateComponent.mPlayerPokemonRoster.size())
    {
        auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
        if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
        {
            encounterStateComponent.mEncounterJustFinished = true;
        }
        else
        {
            //TODO: Continue with overworld
        }
    }

    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        //CompleteAndTransitionTo<EvolutionAnimationFlowState>
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

