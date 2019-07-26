//
//  EncounterStateControllerSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 24/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "EncounterStateControllerSystem.h"
#include "../components/EncounterStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/flowstates/DarkenedOpponentsIntroEncounterFlowState.h"
#include "../../common/utils/PokemonUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../overworld/components/LevelModelComponent.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../overworld/utils/LevelLoadingUtils.h"
#include "../../overworld/utils/OverworldUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

EncounterStateControllerSystem::EncounterStateControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    InitializeEncounterState();
}

void EncounterStateControllerSystem::VUpdateAssociatedComponents(const float dt) const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
        
    // Battle finished condition
    if (encounterStateComponent.mEncounterJustFinished)
    {
        encounterStateComponent.mEncounterJustFinished = false;
    }    
    // Battle ongoing condition
    else if (encounterStateComponent.mFlowStateManager.HasActiveFlowState())
    {
        encounterStateComponent.mFlowStateManager.Update(dt);
    }
    // Battle started condition
    else if (encounterStateComponent.mOverworldEncounterAnimationState == OverworldEncounterAnimationState::ENCOUNTER_INTRO_ANIMATION_COMPLETE)
    {                
        const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
        const auto& levelModelComponent  = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
        
        DestroyLevel(levelModelComponent.mLevelName, mWorld);
        mWorld.RemoveEntity(GetPlayerEntityId(mWorld));
        
        const auto newLevelEntityId        = LoadAndCreateLevelByName(StringId("battle"), mWorld);
        auto& encounterLevelModelComponent = mWorld.GetComponent<LevelModelComponent>(newLevelEntityId);
        
        mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>().mActiveLevelNameId = encounterLevelModelComponent.mLevelName;
     
        auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();        

        encounterStateComponent.mNumberOfEscapeAttempts = 0;
        encounterStateComponent.mHasEscapeSucceeded     = false;
        encounterStateComponent.mOverworldEncounterAnimationState = OverworldEncounterAnimationState::NONE;
        encounterStateComponent.mFlowStateManager.SetActiveFlowState(std::make_unique<DarkenedOpponentsIntroEncounterFlowState>(mWorld));
        encounterStateComponent.mActivePlayerPokemonRosterIndex = GetFirstNonFaintedPokemonIndex(playerStateComponent.mPlayerPokemonRoster);
        encounterStateComponent.mPlayerPokemonToOpponentPokemonDamageMap.clear();

        // Reset all stat modifiers for all player's pokemon        
        for (auto i = 0U; i < playerStateComponent.mPlayerPokemonRoster.size(); ++i)
        {
            ResetPokemonEncounterModifierStages(*playerStateComponent.mPlayerPokemonRoster[i]);
            for (auto j = 0U; j < encounterStateComponent.mOpponentPokemonRoster.size(); ++j)
            {
                encounterStateComponent.mPlayerPokemonToOpponentPokemonDamageMap[i][j] = 0.0f;
            }
        }

        CreateChatbox(mWorld);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void EncounterStateControllerSystem::InitializeEncounterState() const
{    
    mWorld.SetSingletonComponent<EncounterStateSingletonComponent>(std::make_unique<EncounterStateSingletonComponent>());
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

