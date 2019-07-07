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
#include "../../encounter/utils/EncounterSpriteUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PlayerPokemonSummonEncounterFlowState::PLAYER_STATUS_DISPLAY_POSITION    = glm::vec3(0.3f, -0.23f, 0.0f);
const glm::vec3 PlayerPokemonSummonEncounterFlowState::PLAYER_HEALTHBAR_DISPLAY_POSITION = glm::vec3(0.3f, -0.23f, 0.1f);
const glm::vec3 PlayerPokemonSummonEncounterFlowState::PLAYER_STATUS_DISPLAY_SCALE       = glm::vec3(1.0f, 1.0f, 1.0f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PlayerPokemonSummonEncounterFlowState::PlayerPokemonSummonEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    // TODO: differentiate between PIKACHU summon and other pokemon anim
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    // Player pokemon status display
    encounterStateComponent.mViewEntities.mPlayerStatusDisplayEntityId = LoadAndCreatePlayerPokemonStatusDisplay
    (
        PLAYER_STATUS_DISPLAY_POSITION,
        PLAYER_STATUS_DISPLAY_SCALE,
        mWorld
    );
    
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

