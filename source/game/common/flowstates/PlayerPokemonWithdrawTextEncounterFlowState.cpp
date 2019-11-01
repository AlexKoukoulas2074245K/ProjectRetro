//
//  PlayerPokemonWithdrawTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PlayerPokemonWithdrawTextEncounterFlowState.h"
#include "PlayerPokemonTextIntroEncounterFlowState.h"
#include "PokemonScalingAnimationEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/components/PokemonSpriteScalingAnimationStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerPokemonWithdrawTextEncounterFlowState::PlayerPokemonWithdrawTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto mainChatboxEntityId = CreateChatbox(world);
    
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    const auto maxOpponentPokemonHp       = encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex]->mMaxHp;
    const auto withdrawnPokemonDamageDone = encounterStateComponent.mPlayerPokemonToOpponentPokemonDamageMap
    [encounterStateComponent.mActivePlayerPokemonRosterIndex]
    [encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    if (withdrawnPokemonDamageDone <= 0.01f)
    {
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mName.GetString() + " enough!#Come back!+FREEZE",
            mWorld
        );
    }    
    else if (withdrawnPokemonDamageDone / maxOpponentPokemonHp <= 0.25f)
    {
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mName.GetString() + "#Come back!+FREEZE",
            mWorld
        );
    }
    else if (withdrawnPokemonDamageDone / maxOpponentPokemonHp <= 0.5f)
    {
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mName.GetString() + " OK!#Come back!+FREEZE",
            mWorld
        );
    }
    else
    {
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mName.GetString() + " good!#Come back!+FREEZE",
            mWorld
        );
    }    
}

void PlayerPokemonWithdrawTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        auto& pokemonSpriteScalingComponent = mWorld.GetSingletonComponent<PokemonSpriteScalingAnimationStateSingletonComponent>();
        pokemonSpriteScalingComponent.mScaleOpponentPokemon = false;
        pokemonSpriteScalingComponent.mScalingAnimationType = ScalingAnimationType::SCALING_DOWN;

        CompleteAndTransitionTo<PokemonScalingAnimationEncounterFlowState>();        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

