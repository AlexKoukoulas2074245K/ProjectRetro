//
//  PokemonDeathMovementEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 17/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonDeathMovementEncounterFlowState.h"
#include "PokemonDeathTextEncounterFlowState.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float PokemonDeathMovementEncounterFlowState::OPPONENT_POKEMON_DEATH_TARGET_Y = 0.0f;
const float PokemonDeathMovementEncounterFlowState::PLAYER_POKEMON_DEATH_TARGET_Y   = -0.24f;
const float PokemonDeathMovementEncounterFlowState::POKEMON_SPRITE_MOVE_SPEED       = 2.0f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokemonDeathMovementEncounterFlowState::PokemonDeathMovementEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    
}

void PokemonDeathMovementEncounterFlowState::VUpdate(const float dt)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster.front();
    auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster.front();
    
    if (activeOpponentPokemon.mHp <= 0)
    {
        auto& transformComponent = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
        transformComponent.mPosition.y -= POKEMON_SPRITE_MOVE_SPEED * dt;
        
        if (transformComponent.mPosition.y <= OPPONENT_POKEMON_DEATH_TARGET_Y)
        {
            transformComponent.mPosition.y = OPPONENT_POKEMON_DEATH_TARGET_Y;
            
            const auto mainChatboxEntityId = CreateChatbox(mWorld);
            QueueDialogForTextbox
            (
                mainChatboxEntityId,
                "Enemy " + activeOpponentPokemon.mName.GetString() + "#fainted!#+END",
                mWorld
            );
            
            mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
            mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId);
            mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId);
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, mWorld);
            
            CompleteAndTransitionTo<PokemonDeathTextEncounterFlowState>();
        }
    }
    else
    {
        auto& transformComponent = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);
        transformComponent.mPosition.y -= POKEMON_SPRITE_MOVE_SPEED * dt;
        
        if (transformComponent.mPosition.y <= PLAYER_POKEMON_DEATH_TARGET_Y)
        {
            transformComponent.mPosition.y = PLAYER_POKEMON_DEATH_TARGET_Y;
            
            const auto mainChatboxEntityId = CreateChatbox(mWorld);
            QueueDialogForTextbox
            (
                mainChatboxEntityId,
                activePlayerPokemon.mName.GetString() + "#fainted!#+END",
                mWorld
            );
            
            mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);
            mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId);
            mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mOpponentPokemonDeathCoverEntityId);
            mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mPlayerPokemonHealthBarEntityId);
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, mWorld);
             
            CompleteAndTransitionTo<PokemonDeathTextEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

