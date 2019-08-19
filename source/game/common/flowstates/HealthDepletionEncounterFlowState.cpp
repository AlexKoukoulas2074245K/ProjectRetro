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
#include "../utils/PokemonUtils.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/Logging.h"
#include "../../common/utils/MathUtils.h"
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
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
        
    if (GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld).mPower == 0)
    {
        encounterStateComponent.mOutstandingFloatDamage = 0.0f;
    }
    
    if (encounterStateComponent.mIsOpponentsTurn)
    {
        if (encounterStateComponent.mDidConfusedPokemonHurtItself == false)
        {
            activePlayerPokemon.mHp -= static_cast<int>(encounterStateComponent.mOutstandingFloatDamage);
        }
        else
        {
            activeOpponentPokemon.mHp -= static_cast<int>(encounterStateComponent.mOutstandingFloatDamage);
        }
        
        
    }
    else
    {
        if (encounterStateComponent.mDidConfusedPokemonHurtItself == false)
        {
            activeOpponentPokemon.mHp -= static_cast<int>(encounterStateComponent.mOutstandingFloatDamage);
        }
        else
        {
            activePlayerPokemon.mHp -= static_cast<int>(encounterStateComponent.mOutstandingFloatDamage);
        }
    }
    
    activePlayerPokemon.mHp   = math::Max(0, activePlayerPokemon.mHp);
    activeOpponentPokemon.mHp = math::Max(0, activeOpponentPokemon.mHp);
    
}

void HealthDepletionEncounterFlowState::VUpdate(const float dt)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld).mPower == 0)
    {
        CompleteAndTransitionTo<MoveEffectivenessTextEncounterFlowState>();
        return;
    }

    encounterStateComponent.mDefenderFloatHealth    -= DEPLETION_SPEED * dt;
    encounterStateComponent.mOutstandingFloatDamage -= DEPLETION_SPEED * dt;

    RefreshHurtPokemonStats();

    if 
    (
        encounterStateComponent.mOutstandingFloatDamage <= 0.0f ||
        encounterStateComponent.mDefenderFloatHealth <= 0.0f
    )
    {
        // End damage calculation
        encounterStateComponent.mOutstandingFloatDamage = 0.0f;

        // Make sure to not show negative damage
        RefreshHurtPokemonStats();

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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void HealthDepletionEncounterFlowState::RefreshHurtPokemonStats() const
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (encounterStateComponent.mIsOpponentsTurn)
    {
        if (encounterStateComponent.mDidConfusedPokemonHurtItself == false)
        {
            RefreshPlayerPokemonStats();
        }
        else
        {
            RefreshOpponentPokemonStats();
        }
    }
    else
    {
        if (encounterStateComponent.mDidConfusedPokemonHurtItself == false)
        {
            RefreshOpponentPokemonStats();
        }
        else
        {
            RefreshPlayerPokemonStats();
        }
    }
}

void HealthDepletionEncounterFlowState::RefreshPlayerPokemonStats() const
{
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();    
    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    
    mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mPlayerPokemonHealthBarEntityId);

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

    auto integerDefendersHealth = static_cast<int>(encounterStateComponent.mDefenderFloatHealth);

    if (integerDefendersHealth == 0 && activePlayerPokemon.mHp > 0)
    {
        integerDefendersHealth = 1;
    }
    
    if
    (
         encounterStateComponent.mOutstandingFloatDamage <= 0.0f ||
         encounterStateComponent.mDefenderFloatHealth <= 0.0f
    )
    {
        WriteTextAtTextboxCoords
        (
            encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
            std::to_string(activePlayerPokemon.mHp) + "/",
            4 - static_cast<int>(std::to_string(integerDefendersHealth).size()),
            3,
            mWorld
        );
    }
    else
    {
        WriteTextAtTextboxCoords
        (
            encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
            std::to_string(math::Max(0, integerDefendersHealth)) + "/",
            4 - static_cast<int>(std::to_string(integerDefendersHealth).size()),
            3,
            mWorld
        );
    }
}

void HealthDepletionEncounterFlowState::RefreshOpponentPokemonStats() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& activeOpponentPokemon   = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId);

    if
    (
         encounterStateComponent.mOutstandingFloatDamage <= 0.0f ||
         encounterStateComponent.mDefenderFloatHealth <= 0.0f
    )
    {
        encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId = LoadAndCreatePokemonHealthBar
        (
            static_cast<float>(activeOpponentPokemon.mHp) / activeOpponentPokemon.mMaxHp,
            true,
            mWorld
        );
    }
    else
    {
        encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId = LoadAndCreatePokemonHealthBar
        (
            encounterStateComponent.mDefenderFloatHealth / activeOpponentPokemon.mMaxHp,
            true,
            mWorld
        );
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
