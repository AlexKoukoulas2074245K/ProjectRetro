//
//  PokemonDeathTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "AwardExperienceEncounterFlowState.h"
#include "PokemonDeathTextEncounterFlowState.h"
#include "PokemonSelectionViewFlowState.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
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
        // Player's pokemon fainted
        else
        { 
            if (encounterStateComponent.mActiveEncounterType == EncounterType::TRAINER)
            {
                auto& pokemonSelectionViewStateComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
                pokemonSelectionViewStateComponent.mCreationSourceType = PokemonSelectionViewCreationSourceType::ENCOUNTER_AFTER_POKEMON_FAINTED;
                CompleteAndTransitionTo<PokemonSelectionViewFlowState>();
            }
            else
            {
                auto& pokemonSelectionViewStateComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
                pokemonSelectionViewStateComponent.mCreationSourceType = PokemonSelectionViewCreationSourceType::ENCOUNTER_AFTER_POKEMON_FAINTED;
                CompleteAndTransitionTo<PokemonSelectionViewFlowState>();
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
