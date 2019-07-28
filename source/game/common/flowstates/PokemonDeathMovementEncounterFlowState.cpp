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
#include "../../common/utils/PokemonUtils.h"
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
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    
    if (activeOpponentPokemon.mHp <= 0)
    {
        auto& transformComponent = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
        transformComponent.mPosition.y -= POKEMON_SPRITE_MOVE_SPEED * dt;
        
        if (transformComponent.mPosition.y <= OPPONENT_POKEMON_DEATH_TARGET_Y)
        {
            transformComponent.mPosition.y = OPPONENT_POKEMON_DEATH_TARGET_Y;
            
            const auto mainChatboxEntityId = CreateChatbox(mWorld);
            QueueDialogForChatbox
            (
                mainChatboxEntityId,
                "Enemy " + activeOpponentPokemon.mName.GetString() + "#fainted!#+END",
                mWorld
            );
            
            mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
            mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId);
            mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId);
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, mWorld);
            
            encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId       = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId      = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId   = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId = ecs::NULL_ENTITY_ID;

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
            QueueDialogForChatbox
            (
                mainChatboxEntityId,
                activePlayerPokemon.mName.GetString() + "#fainted!#+END",
                mWorld
            );
            
            mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);
            mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId);
            mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentPokemonDeathCoverEntityId);
            mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mPlayerPokemonHealthBarEntityId);
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, mWorld);
             
            encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId        = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mPlayerStatusDisplayEntityId       = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mOpponentPokemonDeathCoverEntityId = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mPlayerPokemonHealthBarEntityId    = ecs::NULL_ENTITY_ID;
            encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId  = ecs::NULL_ENTITY_ID;

            CompleteAndTransitionTo<PokemonDeathTextEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

