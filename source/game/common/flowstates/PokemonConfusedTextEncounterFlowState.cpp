//
//  PokemonConfusedTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 15/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonConfusedTextEncounterFlowState.h"
#include "ConfusionAnimationEncounterFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokemonConfusedTextEncounterFlowState::PokemonConfusedTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    const auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    std::string confusionText = "";
    if (encounterStateComponent.mIsOpponentsTurn)
    {
        confusionText += "Enemy " + activeOpponentPokemon.mName.GetString() + "#is confused!#+END";
    }
    else
    {
        confusionText += activePlayerPokemon.mName.GetString() + "#is confused!#+END";
    }
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox(mainChatboxEntityId, confusionText, mWorld);
}

void PokemonConfusedTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    const auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        const auto mainChatboxEntityId = CreateChatbox(mWorld);

        if (encounterStateComponent.mIsOpponentsTurn)
        {
            WriteTextAtTextboxCoords(mainChatboxEntityId, "Enemy " + activeOpponentPokemon.mName.GetString(), 1, 2, mWorld);
            WriteTextAtTextboxCoords(mainChatboxEntityId, "is confused!", 1, 4, mWorld);
        }
        else
        {
            WriteTextAtTextboxCoords(mainChatboxEntityId, activePlayerPokemon.mName.GetString(), 1, 2, mWorld);
            WriteTextAtTextboxCoords(mainChatboxEntityId, "is confused!", 1, 4, mWorld);
        }

        CompleteAndTransitionTo<ConfusionAnimationEncounterFlowState>();
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

