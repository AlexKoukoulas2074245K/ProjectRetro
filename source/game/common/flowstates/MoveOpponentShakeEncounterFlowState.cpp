//
//  MoveOpponentShakeEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MoveOpponentShakeEncounterFlowState.h"
#include "../utils/PokemonMoveUtils.h"
#include "../../encounter/components/EncounterShakeSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../rendering/components/CameraSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MoveOpponentShakeEncounterFlowState::MoveOpponentShakeEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    DetermineShakeTypeToBeInitiated();
}

void MoveOpponentShakeEncounterFlowState::VUpdate(const float)
{
    const auto& shakeComponent = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();

    // Wait until Shake is finished
    if (shakeComponent.mActiveShakeType == ShakeType::NONE)
    {        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void MoveOpponentShakeEncounterFlowState::DetermineShakeTypeToBeInitiated() const
{
    const auto& encounterComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& lastMoveUsedStats  = GetMoveStats(encounterComponent.mLastMoveSelected, mWorld);
    const auto isOpponentsTurn     = encounterComponent.mIsOpponentsTurn;
    auto& shakeComponent           = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();

    if (isOpponentsTurn == false)
    {
        if (lastMoveUsedStats.mEffect == StringId())
        {
            shakeComponent.mActiveShakeType = ShakeType::OPPONENT_POKEMON_BLINK;
        }
        else if (lastMoveUsedStats.mEffect != StringId() && lastMoveUsedStats.mPower == 0)
        {
            shakeComponent.mActiveShakeType = ShakeType::OPPONENT_POKEMON_LONG_HORIZONTAL_SHAKE;
        }
        else if (lastMoveUsedStats.mEffect != StringId() && lastMoveUsedStats.mPower > 0)
        {
            shakeComponent.mActiveShakeType = ShakeType::OPPONENT_POKEMON_SHORT_HORIZONTAL_SHAKE;
        }
    }
    else
    {
        if (lastMoveUsedStats.mEffect == StringId())
        {
            shakeComponent.mActiveShakeType = ShakeType::PLAYER_POKEMON_VERTICAL_SHAKE;
        }
        else if (lastMoveUsedStats.mEffect != StringId() && lastMoveUsedStats.mPower == 0)
        {
            shakeComponent.mActiveShakeType = ShakeType::PLAYER_POKEMON_LONG_HORIZONTAL_SHAKE;
        }
        else if (lastMoveUsedStats.mEffect != StringId() && lastMoveUsedStats.mPower > 0)
        {
            shakeComponent.mActiveShakeType = ShakeType::PLAYER_RAPID_LONG_HORIZONTAL_SHAKE;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
