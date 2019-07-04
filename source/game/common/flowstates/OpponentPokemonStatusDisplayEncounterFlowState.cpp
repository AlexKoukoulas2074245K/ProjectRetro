//
//  OpponentPokemonStatusDisplayEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OpponentPokemonStatusDisplayEncounterFlowState.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 OpponentPokemonStatusDisplayEncounterFlowState::OPPONENT_STATUS_DISPLAY_POSITION       = glm::vec3(-0.32f, 0.7f, 0.0f);
const glm::vec3 OpponentPokemonStatusDisplayEncounterFlowState::OPPONENT_STATUS_DISPLAY_SCALE          = glm::vec3(1.0f, 1.0f, 1.0f);
const glm::vec3 OpponentPokemonStatusDisplayEncounterFlowState::OPPONENT_POKEMON_INFO_TEXTBOX_POSITION = glm::vec3(0.04f, 0.858f, -0.5f);

const int OpponentPokemonStatusDisplayEncounterFlowState::OPPONENT_POKEMON_INFO_TEXTBOX_COLS = 20;
const int OpponentPokemonStatusDisplayEncounterFlowState::OPPONENT_POKEMON_INFO_TEXTBOX_ROWS = 2;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OpponentPokemonStatusDisplayEncounterFlowState::OpponentPokemonStatusDisplayEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    encounterStateComponent.mOpponentStatusDisplayEntityId = LoadAndCreateOpponentPokemonStatusDisplay
    (
        encounterStateComponent.mOpponentPokemonRoster.front(),
        OPPONENT_STATUS_DISPLAY_POSITION,
        OPPONENT_STATUS_DISPLAY_SCALE,
        mWorld
    );
    
    // Create opponent pokemon name and level textbox
    encounterStateComponent.mOpponentPokemonInfoTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::BARE_TEXTBOX,
        OPPONENT_POKEMON_INFO_TEXTBOX_COLS,
        OPPONENT_POKEMON_INFO_TEXTBOX_ROWS,
        OPPONENT_POKEMON_INFO_TEXTBOX_POSITION.x,
        OPPONENT_POKEMON_INFO_TEXTBOX_POSITION.y,
        OPPONENT_POKEMON_INFO_TEXTBOX_POSITION.z,
        mWorld
    );
    
    // Write opponent pokemon name string
    const auto opponentPokemonName = encounterStateComponent.mOpponentPokemonRoster.front().mName.GetString();
    WriteTextAtTextboxCoords(encounterStateComponent.mOpponentPokemonInfoTextboxEntityId, opponentPokemonName, 0, 0, mWorld);
    
    // Write opponent pokemon level string
    const auto opponentPokemonLevel = encounterStateComponent.mOpponentPokemonRoster.front().mLevel;
    WriteTextAtTextboxCoords(encounterStateComponent.mOpponentPokemonInfoTextboxEntityId, std::to_string(opponentPokemonLevel), 4, 1, mWorld);
}

void OpponentPokemonStatusDisplayEncounterFlowState::VUpdate(const float)
{
    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

