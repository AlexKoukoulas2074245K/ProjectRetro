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

HealthDepletionEncounterFlowState::HealthDepletionEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    if (GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld).mPower == 0)
    {
        encounterStateComponent.mOutstandingFloatDamage = 0.0f;
    }
    
    auto& defendingPokemon = GetDefendingPokemon();

    defendingPokemon.mHp -= static_cast<int>(encounterStateComponent.mOutstandingFloatDamage);
    defendingPokemon.mHp = math::Max(0, defendingPokemon.mHp);        
}

void HealthDepletionEncounterFlowState::VUpdate(const float dt)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld).mPower == 0)
    {
        CompleteAndTransitionTo<MoveEffectivenessTextEncounterFlowState>();
        return;
    }

    const auto healthDepletionSpeed = CalculateHealthDepletionSpeed();

    encounterStateComponent.mDefenderFloatHealth    -= healthDepletionSpeed * dt;
    encounterStateComponent.mOutstandingFloatDamage -= healthDepletionSpeed * dt;

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

Pokemon& HealthDepletionEncounterFlowState::GetDefendingPokemon() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    if (encounterStateComponent.mIsOpponentsTurn)
    {
        if
        (
            encounterStateComponent.mLastMoveSelected != CONFUSION_HURT_ITSELF_MOVE_NAME &&
            encounterStateComponent.mLastMoveSelected != POISON_TICK_MOVE_NAME
        )
        {
            return *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
        }
        else
        {
            return *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
        }                
    }
    else
    {
        if
        (
            encounterStateComponent.mLastMoveSelected != CONFUSION_HURT_ITSELF_MOVE_NAME &&
            encounterStateComponent.mLastMoveSelected != POISON_TICK_MOVE_NAME
        )
        {
            return *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
        }
        else
        {
            return *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
        }
    }
}

float HealthDepletionEncounterFlowState::CalculateHealthDepletionSpeed() const
{    
    const auto& defendingPokemon = GetDefendingPokemon();

    // Depletion of a full hp bar is approximately linear to the parametric t from 1.5secs to 3.0secs
    // based on the pokemon level
    const auto targetDelayInSecsUntilFullHpIsDepleted = math::Lerp(1.5f, 3.0f, defendingPokemon.mLevel / 100.0f);
    return defendingPokemon.mMaxHp / targetDelayInSecsUntilFullHpIsDepleted;
}

void HealthDepletionEncounterFlowState::RefreshHurtPokemonStats() const
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (encounterStateComponent.mIsOpponentsTurn)
    {
        if
        (
            encounterStateComponent.mLastMoveSelected != CONFUSION_HURT_ITSELF_MOVE_NAME &&
            encounterStateComponent.mLastMoveSelected != POISON_TICK_MOVE_NAME
        )
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
        if
        (
            encounterStateComponent.mLastMoveSelected != CONFUSION_HURT_ITSELF_MOVE_NAME &&
            encounterStateComponent.mLastMoveSelected != POISON_TICK_MOVE_NAME
        )
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
            4 - static_cast<int>(std::to_string(activePlayerPokemon.mHp).size()),
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
