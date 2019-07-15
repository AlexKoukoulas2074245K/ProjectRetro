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
#include "PlayerPokemonTextIntroEncounterFlowState.h"
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 OpponentPokemonStatusDisplayEncounterFlowState::OPPONENT_STATUS_DISPLAY_POSITION       = glm::vec3(-0.32f, 0.7f, 0.05f);
const glm::vec3 OpponentPokemonStatusDisplayEncounterFlowState::OPPONENT_STATUS_DISPLAY_SCALE          = glm::vec3(1.0f, 1.0f, 1.0f);
const glm::vec3 OpponentPokemonStatusDisplayEncounterFlowState::OPPONENT_POKEMON_INFO_TEXTBOX_POSITION = glm::vec3(0.04f, 0.858f, -0.5f);
const glm::vec3 OpponentPokemonStatusDisplayEncounterFlowState::PLAYER_TRAINER_EXIT_TARGET_POSITION    = glm::vec3(-1.0f, 0.61f, 0.1f);

const int OpponentPokemonStatusDisplayEncounterFlowState::OPPONENT_POKEMON_INFO_TEXTBOX_COLS = 20;
const int OpponentPokemonStatusDisplayEncounterFlowState::OPPONENT_POKEMON_INFO_TEXTBOX_ROWS = 2;

const float OpponentPokemonStatusDisplayEncounterFlowState::SPRITE_ANIMATION_SPEED = 2.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OpponentPokemonStatusDisplayEncounterFlowState::OpponentPokemonStatusDisplayEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent     = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster.front();
    
    encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId = LoadAndCreateOpponentPokemonStatusDisplay
    (
        OPPONENT_STATUS_DISPLAY_POSITION,
        OPPONENT_STATUS_DISPLAY_SCALE,
        mWorld
    );
    
    //TODO: select appropriate bar color
    encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId = LoadAndCreatePokemonHealthBar
    (
        static_cast<float>(activeOpponentPokemon.mHp)/activeOpponentPokemon.mMaxHp,
        true,
        mWorld
    );

    //TODO: displace health bar appropriately


    // Create opponent pokemon name and level textbox
    encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId = CreateTextboxWithDimensions
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
    const auto opponentPokemonName = activeOpponentPokemon.mName.GetString();
    WriteTextAtTextboxCoords(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, opponentPokemonName, 0, 0, mWorld);
    
    // Write opponent pokemon level string
    const auto opponentPokemonLevel = activeOpponentPokemon.mLevel;
    WriteTextAtTextboxCoords(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, "=" + std::to_string(opponentPokemonLevel), 3, 1, mWorld);
}

void OpponentPokemonStatusDisplayEncounterFlowState::VUpdate(const float dt)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    auto& playerTrainerSpriteTransformComponent = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);
    
    playerTrainerSpriteTransformComponent.mPosition.x -= SPRITE_ANIMATION_SPEED * dt;    
    if (playerTrainerSpriteTransformComponent.mPosition.x < PLAYER_TRAINER_EXIT_TARGET_POSITION.x)
    {
        playerTrainerSpriteTransformComponent.mPosition.x = PLAYER_TRAINER_EXIT_TARGET_POSITION.x;        
        CompleteAndTransitionTo<PlayerPokemonTextIntroEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

