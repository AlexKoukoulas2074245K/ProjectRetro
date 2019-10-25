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
#include "EvolutionTextFlowState.h"
#include "NextOpponentPokemonCheckEncounterFlowState.h"
#include "TrainerBattleWonEncounterFlowState.h"
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
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    std::sort(encounterStateComponent.mPlayerPokemonIndicesEligibleForXp.begin(), encounterStateComponent.mPlayerPokemonIndicesEligibleForXp.end());
    
    const auto& playerStateComponent  = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];
    
    auto& activePlayerPokemon = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mPlayerPokemonIndicesEligibleForXp.front()];
    
    if (activePlayerPokemon.mLevel < 100)
    {
        //TODO: account for multiple pokemon participating
        const auto xpAwarded = CalculateXpGainFromBattle
        (
            encounterStateComponent.mActiveEncounterType == EncounterType::WILD,
            activeOpponentPokemon.mBaseSpeciesStats.mXpStat,
            activeOpponentPokemon.mLevel,
            static_cast<int>(encounterStateComponent.mNumberOfPlayerPokemonEligibleForXp)
        );

        activePlayerPokemon.mXpPoints  += xpAwarded;
        AddToEvStat(activeOpponentPokemon.mBaseSpeciesStats.mAttack, activePlayerPokemon.mAttackEv);
        AddToEvStat(activeOpponentPokemon.mBaseSpeciesStats.mDefense, activePlayerPokemon.mDefenseEv);
        AddToEvStat(activeOpponentPokemon.mBaseSpeciesStats.mSpeed, activePlayerPokemon.mSpeedEv);
        AddToEvStat(activeOpponentPokemon.mBaseSpeciesStats.mSpecial, activePlayerPokemon.mSpecialEv);
        
        const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
        if (guiStateComponent.mActiveTextboxesStack.size() == 2)
        {
            DestroyActiveTextbox(mWorld);
        }

        const auto mainChatboxEntityId = CreateChatbox(mWorld);
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            activePlayerPokemon.mName.GetString() + " gained#" + std::to_string(xpAwarded) + " EXP. Points!#+END",
            mWorld
        );
    }        
}

void AwardExperienceEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent   = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& playerStateComponent      = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent   = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& activePlayerPokemon = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mPlayerPokemonIndicesEligibleForXp.front()];
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        const auto totalXpNeededForNextLevel = CalculatePokemonTotalExperienceAtLevel
        (
            activePlayerPokemon.mBaseSpeciesStats.mSpeciesName,
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
            // There are more pokemon to award xp to
            if (encounterStateComponent.mPlayerPokemonIndicesEligibleForXp.size() > 1)
            {
                encounterStateComponent.mPlayerPokemonIndicesEligibleForXp.erase(encounterStateComponent.mPlayerPokemonIndicesEligibleForXp.begin());
                CompleteAndTransitionTo<AwardExperienceEncounterFlowState>();
            }
            // There aren't any more pokemon to award xp to
            else
            {
                encounterStateComponent.mPlayerPokemonIndicesEligibleForXp.clear();
                if (encounterStateComponent.mActiveEncounterType == EncounterType::WILD)
                {
                    CompleteAndTransitionTo<EvolutionTextFlowState>();
                }
                else
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
            }            
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
