//
//  OpponentIntroTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OpponentIntroTextEncounterFlowState.h"
#include "OpponentPokemonStatusDisplayEncounterFlowState.h"
#include "../components/EncounterStateSingletonComponent.h"
#include "../utils/EncounterSpriteUtils.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string OpponentIntroTextEncounterFlowState::POKEMON_BATTLE_SPRITE_MODEL_NAME = "pokemon_battle_sprite";

const glm::vec3 OpponentIntroTextEncounterFlowState::PLAYER_ROSTER_DISPLAY_POSITION       = glm::vec3(0.3f, -0.23f, 0.0f);
const glm::vec3 OpponentIntroTextEncounterFlowState::PLAYER_ROSTER_DISPLAY_SCALE          = glm::vec3(1.1f, 1.1f, 1.0f);
const glm::vec3 OpponentIntroTextEncounterFlowState::PLAYER_POKEMON_INFO_TEXTBOX_POSITION = glm::vec3(0.7225f, -0.025f, -0.5f);

const int OpponentIntroTextEncounterFlowState::PLAYER_POKEMON_INFO_TEXTBOX_COLS = 20;
const int OpponentIntroTextEncounterFlowState::PLAYER_POKEMON_INFO_TEXTBOX_ROWS = 4;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OpponentIntroTextEncounterFlowState::OpponentIntroTextEncounterFlowState(ecs::World& world)
    : BaseEncounterFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    encounterStateComponent.mPlayerStatusDisplayEntityId = LoadAndCreatePlayerRosterDisplay
    (
        PLAYER_ROSTER_DISPLAY_POSITION,
        PLAYER_ROSTER_DISPLAY_SCALE,
        mWorld
    );
    
    // Pokemon party pokeball rendering
    encounterStateComponent.mPlayerPokemonInfoTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::BARE_TEXTBOX,
        PLAYER_POKEMON_INFO_TEXTBOX_COLS,
        PLAYER_POKEMON_INFO_TEXTBOX_ROWS,
        PLAYER_POKEMON_INFO_TEXTBOX_POSITION.x,
        PLAYER_POKEMON_INFO_TEXTBOX_POSITION.y,
        PLAYER_POKEMON_INFO_TEXTBOX_POSITION.z,
        mWorld
    );
    
    const auto& playerPokemonRoster = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>().mPlayerPokemonRoster;
    for (auto i = 0U; i < playerPokemonRoster.size(); ++i)
    {
        // select normal pokeball, status or faint
        WriteTextAtTextboxCoords
        (
            encounterStateComponent.mPlayerPokemonInfoTextboxEntityId,
            "~",
            1 + i,
            3,
            mWorld
        );
    }
    
    
    const auto overlaidChatboxEntityId  = CreateChatbox(mWorld);
    
    if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
    {
        const auto wildPokemonName = encounterStateComponent.mOpponentPokemonRoster.front().mName;
        QueueDialogForTextbox(overlaidChatboxEntityId, "Wild " + wildPokemonName.GetString() + "#appeared!#END", mWorld);
    }
}

void OpponentIntroTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        mWorld.RemoveEntity(encounterStateComponent.mPlayerStatusDisplayEntityId);
        DeleteTextAtTextboxRow(encounterStateComponent.mPlayerPokemonInfoTextboxEntityId, 3, mWorld);
        encounterStateComponent.mPlayerStatusDisplayEntityId = ecs::NULL_ENTITY_ID;
        CompleteAndTransitionTo<OpponentPokemonStatusDisplayEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
