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
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
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
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId = LoadAndCreatePlayerRosterDisplay
    (
        PLAYER_ROSTER_DISPLAY_POSITION,
        PLAYER_ROSTER_DISPLAY_SCALE,
        mWorld
    );
    
    // Pokemon party pokeball rendering
    encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId = CreateTextboxWithDimensions
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
            encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
            "~", // Pokeball status icon character
            1 + i,
            3,
            mWorld
        );
    }
    
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    
    if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
    {
        const auto wildPokemonName = encounterStateComponent.mOpponentPokemonRoster.front().mName;
        QueueDialogForTextbox
        (
            mainChatboxEntityId,
            "Wild " + wildPokemonName.GetString() + "#appeared!#+END", 
            mWorld
        );
    }
}

void OpponentIntroTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {        
        mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId);
        DeleteTextAtTextboxRow(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 3, mWorld);
        encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId = ecs::NULL_ENTITY_ID;
        CompleteAndTransitionTo<OpponentPokemonStatusDisplayEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
