//
//  PokemonJustWokeUpEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 20/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TurnOverEncounterFlowState.h"
#include "PokemonJustWokeUpEncounterFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../utils/PokemonUtils.h"

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
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activePlayerPokemon     = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon   = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        if (encounterStateComponent.mIsOpponentsTurn == false)
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
                "=" + std::to_string(activePlayerPokemon.mLevel),
                4,
                1,
                mWorld
            );
        }
        else
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
                "=" + std::to_string(activeOpponentPokemon.mLevel),
                3,
                1,
                mWorld
            );
        }
        
        CompleteAndTransitionTo<TurnOverEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
