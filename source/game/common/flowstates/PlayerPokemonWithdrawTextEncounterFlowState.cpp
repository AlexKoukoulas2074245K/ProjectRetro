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
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerPokemonWithdrawTextEncounterFlowState::PlayerPokemonWithdrawTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto mainChatboxEntityId = CreateChatbox(world);

    const auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    const auto& playerStateComponent          = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent             = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    // Switch first step Random?
    // POKEMONNAME enough!#Come back! no damage done to opponents pokemon hp
    // POKEMONNAME good!#Come back! > 0.75 opponents pokemon hp damange done
    // POKEMONNAME OK!#Come back! > 0.5 opponents pokemon hp damange done
    // POKEMONNAME#Come back! > 0.25 opponents pokemon hp damange done
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

    encounterStateComponent.mActivePlayerPokemonRosterIndex = pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex;
}

void PlayerPokemonWithdrawTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);
        encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId = ecs::NULL_ENTITY_ID;

        DestroyActiveTextbox(mWorld);
        CompleteAndTransitionTo<PlayerPokemonTextIntroEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

