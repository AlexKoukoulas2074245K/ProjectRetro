//
//  PokemonFastAsleepTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 20/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonFastAsleepTextEncounterFlowState.h"
#include "FastAsleepAnimationEncounterFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokemonFastAsleepTextEncounterFlowState::PokemonFastAsleepTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    const auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    
    // Restore selected move pp
    auto& playerSelectedMove = *activePlayerPokemon.mMoveSet[encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu];
    playerSelectedMove.mPowerPointsLeft++;

    std::string asleepText = "";
    if (encounterStateComponent.mIsOpponentsTurn)
    {
        asleepText += "Enemy " + activeOpponentPokemon.mName.GetString() + "#is fast asleep!#+END";
    }
    else
    {
        asleepText += activePlayerPokemon.mName.GetString() + "#is fast asleep!#+END";
    }
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox(mainChatboxEntityId, asleepText, mWorld);
}

void PokemonFastAsleepTextEncounterFlowState::VUpdate(const float)
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
            WriteTextAtTextboxCoords(mainChatboxEntityId, "is fast asleep!", 1, 4, mWorld);
        }
        else
        {
            WriteTextAtTextboxCoords(mainChatboxEntityId, activePlayerPokemon.mName.GetString(), 1, 2, mWorld);
            WriteTextAtTextboxCoords(mainChatboxEntityId, "is fast asleep!", 1, 4, mWorld);
        }
        
        CompleteAndTransitionTo<FastAsleepAnimationEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
