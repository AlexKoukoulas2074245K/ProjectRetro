//
//  LearnNewMoveFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "EvolutionTextFlowState.h"
#include "LearnNewMoveFlowState.h"
#include "NextOpponentPokemonCheckEncounterFlowState.h"
#include "TrainerBattleWonEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/components/EvolutionAnimationStateSingletonComponent.h"
#include "../../common/utils/PokemonUtils.h"
#include "../../common/utils/PokemonMoveUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../sound/SoundService.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string LearnNewMoveFlowState::POKEMON_LEVEL_UP_SFX_NAME = "general/level_up_or_badge";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

LearnNewMoveFlowState::LearnNewMoveFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& activePlayerPokemon     = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];
    const auto& moveStats         = GetMoveStats(activePlayerPokemon.mMoveToBeLearned, world);
    
    playerStateComponent.mLeveledUpPokemonRosterIndex = -1;
    
    activePlayerPokemon.mMoveSet[activePlayerPokemon.mMovesetIndexForNewMove] = std::make_unique<PokemonMoveStats>
    (
        moveStats.mName,
        moveStats.mType,
        moveStats.mEffect,
        moveStats.mPower,
        moveStats.mAccuracy,
        moveStats.mTotalPowerPoints
    );
    
    activePlayerPokemon.mMoveToBeLearned = StringId();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        DestroyActiveTextbox(mWorld);
    }
    
    const auto mainChatboxEntityId = CreateChatbox(world);
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        activePlayerPokemon.mName.GetString() + " learned#" + moveStats.mName.GetString() + "!#+FREEZE",
        mWorld
    );
}

void LearnNewMoveFlowState::VUpdate(const float)
{
    // Sfx currently playing
    if (SoundService::GetInstance().IsPlayingSfx())
    {
        return;
    }
    // Sfx just finished playing
    else if (WasSfxPlayingOnPreviousUpdate() && SoundService::GetInstance().GetLastPlayedSfxName() == POKEMON_LEVEL_UP_SFX_NAME)
    {
        auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
        auto& evolutionStateComponent = mWorld.GetSingletonComponent<EvolutionAnimationStateSingletonComponent>();
        auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

        playerStateComponent.mLeveledUpPokemonRosterIndex = -1;

        if (evolutionStateComponent.mNeedToCheckEvolutionNewMoves)
        {
            evolutionStateComponent.mNeedToCheckEvolutionNewMoves = false;
            encounterStateComponent.mEncounterJustFinished = true;
        }
        else if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
        {
            CompleteAndTransitionTo<EvolutionTextFlowState>();
        }
        else if (encounterStateComponent.mActiveEncounterType == EncounterType::TRAINER)
        {
            if (GetFirstNonFaintedPokemonIndex(encounterStateComponent.mOpponentPokemonRoster) != encounterStateComponent.mOpponentPokemonRoster.size())
            {
                CompleteAndTransitionTo<NextOpponentPokemonCheckEncounterFlowState>();
            }
            else
            {
                CompleteAndTransitionTo<TrainerBattleWonEncounterFlowState>();
            }
        }
        else
        {
            //TODO: Continue with overworld
        }

        return;
    }

    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN && SoundService::GetInstance().IsPlayingSfx() == false)
    {
        SoundService::GetInstance().PlaySfx(POKEMON_LEVEL_UP_SFX_NAME, true, true);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
