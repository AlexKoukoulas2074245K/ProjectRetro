//
//  PlayerPokemonSummonEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PlayerPokemonSummonEncounterFlowState.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerPokemonSummonEncounterFlowState::PlayerPokemonSummonEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    // TODO: differentiate between PIKACHU summon and other pokemon anim
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    // Write player's pokemon name
    WriteTextAtTextboxCoords
    (
        encounterStateComponent.mViewEntities.mPlayerPokemonInfoTextboxEntityId,
        playerStateComponent.mPlayerPokemonRoster.front().mName.GetString(),
        0,
        0,
        mWorld
    );
    
    // Write player's pokemon level
    WriteTextAtTextboxCoords
    (
        encounterStateComponent.mViewEntities.mPlayerPokemonInfoTextboxEntityId,
        "=" + std::to_string(playerStateComponent.mPlayerPokemonRoster.front().mLevel),
        4,
        1,
        mWorld
    );
    
    const auto playersTotalHp = 31;
    const auto playersCurrentHp = 15;
    
    // Write player's pokemon current hp
    WriteTextAtTextboxCoords
    (
        encounterStateComponent.mViewEntities.mPlayerPokemonInfoTextboxEntityId,
        std::to_string(playersCurrentHp) + "/",
        4 - static_cast<int>(std::to_string(playersCurrentHp).size()),
        3,
        mWorld
    );
    
    // Write player's pokemon current hp
    WriteTextAtTextboxCoords
    (
        encounterStateComponent.mViewEntities.mPlayerPokemonInfoTextboxEntityId,
        std::to_string(playersTotalHp),
        8 - static_cast<int>(std::to_string(playersTotalHp).size()),
        3,
        mWorld
    );
}

void PlayerPokemonSummonEncounterFlowState::VUpdate(const float)
{
    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

