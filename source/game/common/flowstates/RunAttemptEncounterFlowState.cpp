//
//  RunAttemptEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 26/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "RunAttemptEncounterFlowState.h"
#include "MainMenuEncounterFlowState.h"
#include "PoisonTickCheckEncounterFlowState.h"
#include "TurnOverEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/MathUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string RunAttemptEncounterFlowState::RUN_AWAY_SAFELY_SFX_NAME = "encounter/run";

RunAttemptEncounterFlowState::RunAttemptEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    AttemptToRun();
    WriteRunResultText();
}

void RunAttemptEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        if (encounterStateComponent.mActiveEncounterType == EncounterType::TRAINER)
        {
            CompleteAndTransitionTo<MainMenuEncounterFlowState>();
        }
        else
        {
            if (encounterStateComponent.mHasEscapeSucceeded)
            {
                encounterStateComponent.mEncounterJustFinished = true;
            }
            else
            {                
                CompleteAndTransitionTo<PoisonTickCheckEncounterFlowState>();
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void RunAttemptEncounterFlowState::AttemptToRun() const
{
    // https://bulbapedia.bulbagarden.net/wiki/Escape

    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    const auto& playerStateComponent  = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    encounterStateComponent.mNumberOfEscapeAttempts++;

    auto currentPlayerPokemonSpeed   = GetStatWithModifierApplied(activePlayerPokemon.mSpeed, activePlayerPokemon.mSpeedEncounterStage);
    auto currentOpponentPokemonSpeed = GetStatWithModifierApplied(activeOpponentPokemon.mSpeed, activeOpponentPokemon.mSpeedEncounterStage);
    
    if (activePlayerPokemon.mStatus == PokemonStatus::PARALYZED)
    {
        currentPlayerPokemonSpeed /= 4;
    }
    if (activeOpponentPokemon.mStatus == PokemonStatus::PARALYZED)
    {
        currentOpponentPokemonSpeed /= 4;
    }

    if (currentPlayerPokemonSpeed > currentOpponentPokemonSpeed || currentOpponentPokemonSpeed == 0)
    {
        encounterStateComponent.mHasEscapeSucceeded = true;
        return;
    }
    
    const auto escapeFormula = static_cast<int>((currentPlayerPokemonSpeed * 32.0f)/(static_cast<int>((currentOpponentPokemonSpeed / 4.0f)) % 256) + 30 * encounterStateComponent.mNumberOfEscapeAttempts); 

    if (escapeFormula > 255)
    {
        encounterStateComponent.mHasEscapeSucceeded = true;
    }
    else
    {
        const auto rng = math::RandomInt(0, 255);
        encounterStateComponent.mHasEscapeSucceeded = rng < escapeFormula;        
    }
}

void RunAttemptEncounterFlowState::WriteRunResultText() const
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);

    if (encounterStateComponent.mActiveEncounterType == EncounterType::TRAINER)
    {
        QueueDialogForChatbox(mainChatboxEntityId, "No! There's no#running from a#trainer battle!#+END", mWorld);
    }
    else
    { 
        if (encounterStateComponent.mHasEscapeSucceeded)
        {
            QueueDialogForChatbox(mainChatboxEntityId, "Got away safely!# #+END", mWorld);
            SoundService::GetInstance().PlaySfx(RUN_AWAY_SAFELY_SFX_NAME);
        }
        else
        {
            QueueDialogForChatbox(mainChatboxEntityId, "Can't escape!# #+END", mWorld);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
