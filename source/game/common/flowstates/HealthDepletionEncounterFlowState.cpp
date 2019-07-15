//
//  HealthDepletionEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 13/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "CriticalHitTextEncounterFlowState.h"
#include "HealthDepletionEncounterFlowState.h"
#include "MoveEffectivenessTextEncounterFlowState.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/Logging.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../input/utils/InputUtils.h"
#include "../utils/TextboxUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float HealthDepletionEncounterFlowState::DEPLETION_SPEED = 17.5f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

HealthDepletionEncounterFlowState::HealthDepletionEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster.front();
    auto& activePlayerPokemon = *playerStateComponent.mPlayerPokemonRoster.front();    

    if (encounterStateComponent.mIsOpponentsTurn)
    {
        activePlayerPokemon.mHp -= static_cast<int>(encounterStateComponent.mOutstandingFloatDamage);
    }
    else
    {
        activeOpponentPokemon.mHp -= static_cast<int>(encounterStateComponent.mOutstandingFloatDamage);
    }

}

void HealthDepletionEncounterFlowState::VUpdate(const float dt)
{
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& activeOpponentPokemon      = *encounterStateComponent.mOpponentPokemonRoster.front();
    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster.front();        
    
    if (GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld).mPower == 0)
    {
        CompleteAndTransitionTo<MoveEffectivenessTextEncounterFlowState>();
        return;
    }

    encounterStateComponent.mDefenderFloatHealth -= DEPLETION_SPEED * dt;
    encounterStateComponent.mOutstandingFloatDamage -= DEPLETION_SPEED * dt;

    if (encounterStateComponent.mIsOpponentsTurn)
    {
        RefreshPlayerPokemonStats();
    }
    else
    {
        RefreshOpponentPokemonStats();
    }

    if 
    (
        encounterStateComponent.mOutstandingFloatDamage <= 0.0f ||
        encounterStateComponent.mDefenderFloatHealth <= 0.0f
    )
    {
        // End damage calculation
        encounterStateComponent.mOutstandingFloatDamage = 0.0f;

        // Make sure to not show negative damage
        if (encounterStateComponent.mIsOpponentsTurn)
        {
            RefreshPlayerPokemonStats();
        }
        else
        {
            RefreshOpponentPokemonStats();
        }

        if (encounterStateComponent.mDefenderFloatHealth <= 0.0f)
        {
            encounterStateComponent.mDefenderFloatHealth = 0.0f;

            // End damage calculation with pokemon death
            if (encounterStateComponent.mIsOpponentsTurn)
            {
                activePlayerPokemon.mHp = 0;
            }
            else
            {
                activeOpponentPokemon.mHp = 0;
            }
        }
        else
        {
            if (encounterStateComponent.mLastMoveCrit)
            {
                CompleteAndTransitionTo<CriticalHitTextEncounterFlowState>();
            }
            else
            {
                CompleteAndTransitionTo<MoveEffectivenessTextEncounterFlowState>();
            }
        }        
    }       
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void HealthDepletionEncounterFlowState::RefreshPlayerPokemonStats() const
{
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();    
    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster.front();
    
    mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mPlayerPokemonHealthBarEntityId);

    encounterStateComponent.mViewObjects.mPlayerPokemonHealthBarEntityId = LoadAndCreatePokemonHealthBar
    (
        encounterStateComponent.mDefenderFloatHealth / activePlayerPokemon.mMaxHp,
        false,
        mWorld
    );


    // Write player's pokemon current hp
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 1, 3, mWorld);
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 2, 3, mWorld);
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 3, 3, mWorld);

    const auto integerDefendersHealth = static_cast<int>(encounterStateComponent.mDefenderFloatHealth);

    WriteTextAtTextboxCoords
    (
        encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
        std::to_string(integerDefendersHealth) + "/",
        4 - static_cast<int>(std::to_string(integerDefendersHealth).size()),
        3,
        mWorld
    );
}

void HealthDepletionEncounterFlowState::RefreshOpponentPokemonStats() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster.front();

    mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId);

    encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId = LoadAndCreatePokemonHealthBar
    (
        encounterStateComponent.mDefenderFloatHealth / activeOpponentPokemon.mMaxHp,
        true,
        mWorld
    );
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////