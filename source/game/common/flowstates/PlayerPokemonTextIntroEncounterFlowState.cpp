//
//  PlayerPokemonTextIntroEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/06/2019.
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

    //TODO: differentiate between summoning dialogs
	// On switch
	// < 0.5 hp Do it! POKEMONNAME!
	// < 0.25 hp The enemy's weak!#Get'm! POKEMONNAME!
	// On faint
	
	// < 0.25 hp Get'm! POKEMONNAME!
	// < 0.5 hp Do it! POKEMONNAME!
	
	// Switch first step Random?
	// POKEMONNAME enough!#Come back! no damage done to opponents pokemon hp
	// POKEMONNAME good!#Come back! > 0.75 opponents pokemon hp damange done
	// POKEMONNAME OK!#Come back! > 0.5 opponents pokemon hp damange done
	// POKEMONNAME#Come back! > 0.25 opponents pokemon hp damange done
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        "GO! " + playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mName.GetString() + "!+FREEZE",
        mWorld
    );
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

