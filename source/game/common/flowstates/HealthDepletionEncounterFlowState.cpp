//
//  HealthDepletionEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 13/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "HealthDepletionEncounterFlowState.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/Logging.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

HealthDepletionEncounterFlowState::HealthDepletionEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    
}

void HealthDepletionEncounterFlowState::VUpdate(const float dt)
{
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& activeOpponentPokemon      = *encounterStateComponent.mOpponentPokemonRoster.front();
    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster.front();
    
    if (GetMoveStats(encounterStateComponent.mLastMoveSelected, mWorld).mPower == 0)
    {
        // Continue to effectiveness text
    }

    encounterStateComponent.mDefenderFloatHealth    -= 10.0f * dt;
    encounterStateComponent.mOutstandingFloatDamage -= 10.0f * dt;
    
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
    if (encounterStateComponent.mOutstandingFloatDamage <= 0.0f)
    {
        // End damage calculation
        encounterStateComponent.mOutstandingFloatDamage = 0.0f;
        
        if (encounterStateComponent.mIsOpponentsTurn)
        {
            activePlayerPokemon.mHp = static_cast<int>(encounterStateComponent.mDefenderFloatHealth);
        }
        else
        {
            activeOpponentPokemon.mHp = static_cast<int>(encounterStateComponent.mDefenderFloatHealth);
        }
        
        const auto b = false;
        (void)b;
    }
    
    if (encounterStateComponent.mIsOpponentsTurn)
    {
        mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mPlayerPokemonHealthBarEntityId);
        
        encounterStateComponent.mViewObjects.mPlayerPokemonHealthBarEntityId = LoadAndCreatePokemonHealthBar
        (
            encounterStateComponent.mDefenderFloatHealth/activePlayerPokemon.mMaxHp,
            false,
            mWorld
        );
    }
    else
    {
        mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId);
        
        encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId = LoadAndCreatePokemonHealthBar
        (
            encounterStateComponent.mDefenderFloatHealth/activeOpponentPokemon.mMaxHp,
            true,
            mWorld
        );
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
