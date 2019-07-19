//
//  PokemonDeathTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonDeathTextEncounterFlowState.h"
#include "AwardExperienceEncounterFlowState.h"
#include "../utils/PokemonUtils.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokemonDeathTextEncounterFlowState::PokemonDeathTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    
}

void PokemonDeathTextEncounterFlowState::VUpdate(const float)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& activeOpponentPokemon   = encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        if (activeOpponentPokemon->mHp <= 0)
        {
            CompleteAndTransitionTo<AwardExperienceEncounterFlowState>();
        }        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////