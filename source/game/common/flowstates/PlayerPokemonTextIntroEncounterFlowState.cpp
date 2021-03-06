//
//  PlayerPokemonTextIntroEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 05/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PlayerPokemonTextIntroEncounterFlowState.h"
#include "PlayerPokemonSummonEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerPokemonTextIntroEncounterFlowState::PlayerPokemonTextIntroEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto mainChatboxEntityId = CreateChatbox(world);

    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activeOpponentPokemon   = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    const auto activeOpponentPokemonHpProportionLeft = activeOpponentPokemon.mHp / static_cast<float>(activeOpponentPokemon.mMaxHp);

    if (activeOpponentPokemonHpProportionLeft <= 0.25f)
    {
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            "Get'm! " + playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mName.GetString() + "!+FREEZE",
            mWorld
        );
    }
    else if (activeOpponentPokemonHpProportionLeft <= 0.5f)
    {
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            "Do it! " + playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mName.GetString() + "!+FREEZE",
            mWorld
        );
    }
    else
    {
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            "GO! " + playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mName.GetString() + "!+FREEZE",
            mWorld
        );
    }    
}

void PlayerPokemonTextIntroEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        CompleteAndTransitionTo<PlayerPokemonSummonEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

