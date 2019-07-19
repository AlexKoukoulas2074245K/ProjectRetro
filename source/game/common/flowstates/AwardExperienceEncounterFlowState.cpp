//
//  AwardExperienceEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "AwardExperienceEncounterFlowState.h"
#include "AwardLevelFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

AwardExperienceEncounterFlowState::AwardExperienceEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activeOpponentPokemon   = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    
    auto& activePlayerPokemon = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    
    if (activePlayerPokemon.mLevel < 100)
    {
        //TODO: account for multiple pokemon participating
        const auto xpAwarded = CalculateXpGainFromBattle
        (
            encounterStateComponent.mActiveEncounterType == EncounterType::WILD,
            activeOpponentPokemon.mBaseStats.mXpStat,
            activeOpponentPokemon.mLevel,
            1
        );

        activePlayerPokemon.mXpPoints  += xpAwarded;
        AddToEvStat(activeOpponentPokemon.mBaseStats.mAttack, activePlayerPokemon.mAttackEv);
        AddToEvStat(activeOpponentPokemon.mBaseStats.mDefense, activePlayerPokemon.mDefenseEv);
        AddToEvStat(activeOpponentPokemon.mBaseStats.mSpeed, activePlayerPokemon.mSpeedEv);
        AddToEvStat(activeOpponentPokemon.mBaseStats.mSpecial, activePlayerPokemon.mSpecialEv);

        activePlayerPokemon.mXpPoints  += 100;

        const auto mainChatboxEntityId = CreateChatbox(mWorld);
        QueueDialogForTextbox
        (
            mainChatboxEntityId,
            activePlayerPokemon.mName.GetString() + " gained#" + std::to_string(xpAwarded) + " EXP. Points!#+END",
            mWorld
        );
    }        
}

void AwardExperienceEncounterFlowState::VUpdate(const float)
{    
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& guiStateComponent       = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& playerStateComponent          = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();    
    const auto& activePlayerPokemon     = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        const auto totalXpNeededForNextLevel = CalculatePokemonTotalExperienceAtLevel
        (
            activePlayerPokemon.mName,
            activePlayerPokemon.mLevel + 1,
            mWorld
        );

        if (activePlayerPokemon.mXpPoints >= totalXpNeededForNextLevel)
        {
            playerStateComponent.mLeveledUpPokemonRosterIndex = GetPokemonRosterIndex(activePlayerPokemon, playerStateComponent.mPlayerPokemonRoster);
            CompleteAndTransitionTo<AwardLevelFlowState>();
        }
        else
        {
            //CompleteAndTransitionTo<NextOpponentPokemonCheckEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
