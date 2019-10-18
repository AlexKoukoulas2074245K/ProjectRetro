//
//  BideUnleashTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 17/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BideUnleashAnimationEncounterFlowState.h"
#include "BideUnleashTextEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/MathUtils.h"
#include "../utils/PokemonMoveUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

BideUnleashTextEncounterFlowState::BideUnleashTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox(mainChatboxEntityId, GetUnleashEnergyText(), mWorld);    
}

void BideUnleashTextEncounterFlowState::VUpdate(const float)
{
//    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        const auto mainChatboxEntityId = CreateChatbox(mWorld);
        const auto unleashEnergyText = GetUnleashEnergyText();
        const auto unleashEnergyTextSplitByHash = StringSplit(unleashEnergyText, '#');

        WriteTextAtTextboxCoords(mainChatboxEntityId, unleashEnergyTextSplitByHash[0], 1, 2, mWorld);
        WriteTextAtTextboxCoords(mainChatboxEntityId, unleashEnergyTextSplitByHash[1], 1, 4, mWorld);

        CompleteAndTransitionTo<BideUnleashAnimationEncounterFlowState>();
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

std::string BideUnleashTextEncounterFlowState::GetUnleashEnergyText() const
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    const auto activePokemonName = encounterStateComponent.mIsOpponentsTurn ?
        encounterStateComponent.mOpponentPokemonRoster.at(encounterStateComponent.mActiveOpponentPokemonRosterIndex)->mName :
        playerStateComponent.mPlayerPokemonRoster.at(encounterStateComponent.mActivePlayerPokemonRosterIndex)->mName;
    
    std::string unleashEnergyText = "";

    if (encounterStateComponent.mIsOpponentsTurn)
    {
        unleashEnergyText += "Enemy ";
    }

    unleashEnergyText += activePokemonName.GetString() + "#unleashed energy!#+END";

    return unleashEnergyText;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
