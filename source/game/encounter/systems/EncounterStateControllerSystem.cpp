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
#include "../flowstates/BaseEncounterFlowState.h"
#include "../flowstates/DarkenedOpponentsIntroEncounterFlowState.h"
#include "../components/EncounterStateSingletonComponent.h"

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
    if (false)
    {

    }    
    else if (encounterStateComponent.mActiveEncounterFlowState != nullptr)
    {
        encounterStateComponent.mActiveEncounterFlowState->VUpdate(dt);

        if (encounterStateComponent.mActiveEncounterFlowState->mNextFlowState != nullptr)
        { 
            encounterStateComponent.mActiveEncounterFlowState = std::move(encounterStateComponent.mActiveEncounterFlowState->mNextFlowState);
        }
    }
    // Battle started condition
    else if (encounterStateComponent.mOverworldEncounterAnimationState == OverworldEncounterAnimationState::ENCOUNTER_INTRO_ANIMATION_COMPLETE)
    {
        encounterStateComponent.mActiveEncounterFlowState = std::make_unique<DarkenedOpponentsIntroEncounterFlowState>(mWorld);
        
        const auto& encounterInfo = SelectRandomWildEncounter(levelModelComponent);
        DestroyLevel(levelModelComponent.mLevelName, mWorld);
        mWorld.RemoveEntity(GetPlayerEntityId(mWorld));
        const auto newLevelEntityId = LoadAndCreateLevelByName(StringId("battle"), mWorld);
        auto& levelModelComponent   = mWorld.GetComponent<LevelModelComponent>(newLevelEntityId);
        mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>().mActiveLevelNameId = levelModelComponent.mLevelName;
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
