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
#include "OpponentTrainerRetreatEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 OpponentIntroTextEncounterFlowState::PLAYER_POKEMON_INFO_TEXTBOX_POSITION = glm::vec3(0.7225f, -0.025f, 0.15f);
const glm::vec3 OpponentIntroTextEncounterFlowState::OPPONENT_INFO_TEXTBOX_POSITION       = glm::vec3(-0.2085f, 0.796f, 0.35f);

const int OpponentIntroTextEncounterFlowState::PLAYER_POKEMON_INFO_TEXTBOX_COLS = 20;
const int OpponentIntroTextEncounterFlowState::PLAYER_POKEMON_INFO_TEXTBOX_ROWS = 4;

const std::string OpponentIntroTextEncounterFlowState::POKEMON_BATTLE_SPRITE_MODEL_NAME = "pokemon_battle_sprite";

const glm::vec3 OpponentIntroTextEncounterFlowState::PLAYER_ROSTER_DISPLAY_POSITION   = glm::vec3(0.3f, -0.23f, 0.2f);
const glm::vec3 OpponentIntroTextEncounterFlowState::PLAYER_ROSTER_DISPLAY_SCALE      = glm::vec3(1.04f, 1.04f, 1.0f);
const glm::vec3 OpponentIntroTextEncounterFlowState::OPPONENT_ROSTER_DISPLAY_POSITION = glm::vec3(-0.32f, 0.7f, 0.4f);
const glm::vec3 OpponentIntroTextEncounterFlowState::OPPONENT_ROSTER_DISPLAY_SCALE    = glm::vec3(1.04f, 1.04f, 1.0f);

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
        const auto& pokemon = *playerPokemonRoster[i];
        
        std::string statusString = "~"; // normal status
        
        if (pokemon.mHp <= 0)
        {
            statusString = "+";
        }
        else if (pokemon.mStatus != PokemonStatus::NORMAL)
        {
            statusString = "`";
        }

        // select normal pokeball, status or faint
        WriteTextAtTextboxCoords
        (
            encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
            statusString,
            1 + i,
            3,
            mWorld
        );
    }
    
    for (auto i = playerPokemonRoster.size(); i < 6; ++i)
    {
        WriteTextAtTextboxCoords
        (
            encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
            "\"",
            1 + i,
            3,
            mWorld
        );
    }

    if (encounterStateComponent.mActiveEncounterType == EncounterType::TRAINER)
    {
        encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId = LoadAndCreateOpponentRosterDisplay
        (
            OPPONENT_ROSTER_DISPLAY_POSITION,
            OPPONENT_ROSTER_DISPLAY_SCALE,
            mWorld
        );

        // Pokemon party pokeball rendering
        encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId = CreateTextboxWithDimensions
        (
            TextboxType::BARE_TEXTBOX,
            20,
            2,
            OPPONENT_INFO_TEXTBOX_POSITION.x,
            OPPONENT_INFO_TEXTBOX_POSITION.y,
            OPPONENT_INFO_TEXTBOX_POSITION.z,
            mWorld
        );

        const auto& opponentPokemonRoster = encounterStateComponent.mOpponentPokemonRoster;
        for (auto i = 0U; i < opponentPokemonRoster.size(); ++i)
        {
            const auto& pokemon = *opponentPokemonRoster[i];

            std::string statusString = "~"; // normal status

            if (pokemon.mHp <= 0)
            {
                statusString = "+";
            }
            else if (pokemon.mStatus != PokemonStatus::NORMAL)
            {
                statusString = "`";
            }

            // select normal pokeball, status or faint
            WriteTextAtTextboxCoords
            (
                encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId,
                statusString,
                10 - i,
                1,
                mWorld
            );
        }
        
        for (auto i = opponentPokemonRoster.size(); i < 6U; ++i)
        {
            // select normal pokeball, status or faint
            WriteTextAtTextboxCoords
            (
                encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId,
                "\"",
                10 - i,
                1,
                mWorld
            );
        }
    }

    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    
    if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
    {
        const auto wildPokemonName = encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex]->mName;
        QueueDialogForChatbox(mainChatboxEntityId, "Wild " + wildPokemonName.GetString() + "#appeared!#+END", mWorld);
    }
    else
    {
        QueueDialogForChatbox(mainChatboxEntityId, encounterStateComponent.mOpponentTrainerName.GetString() + " wants#to fight!#+END", mWorld);
    }
}

void OpponentIntroTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
        {
            mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId);
            
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, mWorld);
            
            encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId      = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId = ecs::NULL_ENTITY_ID;
            
            CompleteAndTransitionTo<OpponentPokemonStatusDisplayEncounterFlowState>();
        }
        else
        {
            mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId);
            mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId);
            
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, mWorld);
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, mWorld);
            
            encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId        = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId      = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId   = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId = ecs::NULL_ENTITY_ID;
            
            CompleteAndTransitionTo<OpponentTrainerRetreatEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
