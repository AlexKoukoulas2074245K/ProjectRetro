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
#include "../components/PokemonSpriteScalingAnimationStateSingletonComponent.h"
#include "../utils/EncounterSpriteUtils.h"
#include "../../common/components/DirectionComponent.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/components/PlayerStateSingletonComponent.h"
#include "../../common/flowstates/DarkenedOpponentsIntroEncounterFlowState.h"
#include "../../common/utils/PokemonUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../common/utils/Timer.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/components/LevelModelComponent.h"
#include "../../overworld/components/TransitionAnimationStateSingletonComponent.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../overworld/utils/LevelLoadingUtils.h"
#include "../../overworld/utils/OverworldCharacterLoadingUtils.h"
#include "../../overworld/utils/OverworldUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float EncounterStateControllerSystem::ENCOUNTER_END_ANIMATION_STEP_DURATION = 0.12f;

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
        DestroyEncounterAndCreateLastPlayedLevel();
    }    
    // Battle ongoing condition
    else if (encounterStateComponent.mFlowStateManager.HasActiveFlowState())
    {
        encounterStateComponent.mFlowStateManager.Update(dt);
    }
    // Battle started condition
    else if (encounterStateComponent.mOverworldEncounterAnimationState == OverworldEncounterAnimationState::ENCOUNTER_INTRO_ANIMATION_COMPLETE)
    {
        DestroyCurrentAndCreateEncounterLevel();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void EncounterStateControllerSystem::InitializeEncounterState() const
{    
    mWorld.SetSingletonComponent<EncounterStateSingletonComponent>(std::make_unique<EncounterStateSingletonComponent>());
    mWorld.SetSingletonComponent<PokemonSpriteScalingAnimationStateSingletonComponent>(std::make_unique<PokemonSpriteScalingAnimationStateSingletonComponent>());
}

void EncounterStateControllerSystem::DestroyCurrentAndCreateEncounterLevel() const
{
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    const auto& levelModelComponent  = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));
    auto& playerStateComponent       = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    const auto overworldPlayerEntityId           = GetPlayerEntityId(mWorld);
    const auto& overworldPlayerMovementComponent = mWorld.GetComponent<MovementStateComponent>(overworldPlayerEntityId);
    
    playerStateComponent.mLastOverworldDirection        = mWorld.GetComponent<DirectionComponent>(overworldPlayerEntityId).mDirection;
    playerStateComponent.mLastOverworldLevelOccupiedCol = overworldPlayerMovementComponent.mCurrentCoords.mCol;
    playerStateComponent.mLastOverworldLevelOccupiedRow = overworldPlayerMovementComponent.mCurrentCoords.mRow;
    
    if (encounterStateComponent.mActiveEncounterType == EncounterType::TRAINER)
    {
        const auto npcTrainerEntityId = GetNpcEntityIdFromLevelIndex(playerStateComponent.mLastNpcLevelIndexSpokenTo, mWorld);
        
        const auto& npcMovementStateComponent = mWorld.GetComponent<MovementStateComponent>(npcTrainerEntityId);
        const auto& npcDirectionComponent     = mWorld.GetComponent<DirectionComponent>(npcTrainerEntityId);
        
        playerStateComponent.mLastEngagedTrainerOccupiedRow = npcMovementStateComponent.mCurrentCoords.mRow;
        playerStateComponent.mLastEngagedTrainerOccupiedCol = npcMovementStateComponent.mCurrentCoords.mCol;
        playerStateComponent.mLastEngagedTrainerDirection   = npcDirectionComponent.mDirection;
    }
    
    DestroyLevel(levelModelComponent.mLevelName, mWorld);
    mWorld.DestroyEntity(GetPlayerEntityId(mWorld));
    
    const auto newLevelEntityId        = LoadAndCreateLevelByName(StringId("battle"), mWorld);
    auto& encounterLevelModelComponent = mWorld.GetComponent<LevelModelComponent>(newLevelEntityId);
    
    mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>().mActiveLevelNameId = encounterLevelModelComponent.mLevelName;
    
    encounterStateComponent.mNumberOfEscapeAttempts                             = 0;
    encounterStateComponent.mHasEscapeSucceeded                                 = false;
    encounterStateComponent.mHasPokemonEvolvedInBattle                          = false;
    encounterStateComponent.mOverworldEncounterAnimationState                   = OverworldEncounterAnimationState::NONE;
    encounterStateComponent.mActivePlayerPokemonRosterIndex                     = GetFirstNonFaintedPokemonIndex(playerStateComponent.mPlayerPokemonRoster);
    encounterStateComponent.mLastEncounterMainMenuActionSelected                = MainMenuActionType::FIGHT;
    encounterStateComponent.mPlayerPokemonToOpponentPokemonDamageMap.clear();    

    // Reset all stat modifiers for all player's pokemon
    for (auto i = 0U; i < playerStateComponent.mPlayerPokemonRoster.size(); ++i)
    {
        auto& pokemon = *playerStateComponent.mPlayerPokemonRoster[i];
        
        pokemon.mNumberOfRoundsUntilConfusionEnds = 0;
        
        if (pokemon.mStatus == PokemonStatus::CONFUSED)
        {
            pokemon.mStatus = PokemonStatus::NORMAL;
        }
        
        ResetPokemonEncounterModifierStages(pokemon);
        
        for (auto j = 0U; j < encounterStateComponent.mOpponentPokemonRoster.size(); ++j)
        {
            encounterStateComponent.mPlayerPokemonToOpponentPokemonDamageMap[i][j] = 0.0f;
        }
    }
        
    CreateChatbox(mWorld);
    
    encounterStateComponent.mFlowStateManager.SetActiveFlowState(std::make_unique<DarkenedOpponentsIntroEncounterFlowState>(mWorld));
}

void EncounterStateControllerSystem::DestroyEncounterAndCreateLastPlayedLevel() const
{
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();    
    const auto& levelModelComponent  = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));

    auto& playerStateComponent              = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent           = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
    
    transitionAnimationStateComponent.mAnimationTimer           = std::make_unique<Timer>(ENCOUNTER_END_ANIMATION_STEP_DURATION);
    transitionAnimationStateComponent.mAnimationProgressionStep = -4;
    transitionAnimationStateComponent.mTransitionAnimationType  = TransitionAnimationType::ENCOUNTER_END;
    transitionAnimationStateComponent.mBlackAndWhiteModeEnabled = false;
    
    if (GetNumberOfNonFaintedPokemonInParty(playerStateComponent.mPlayerPokemonRoster) == 0)
    {
        for (auto& pokemon: playerStateComponent.mPlayerPokemonRoster)
        {
            RestorePokemonStats(*pokemon);
        }
        
        transitionAnimationStateComponent.mAnimationProgressionStep = 4;
        playerStateComponent.mLastOverworldLevelName        = playerStateComponent.mHomeLevelName;
        playerStateComponent.mLastOverworldLevelOccupiedCol = playerStateComponent.mHomeLevelOccupiedCol;
        playerStateComponent.mLastOverworldLevelOccupiedRow = playerStateComponent.mHomeLevelOccupiedRow;
    }
    else if (encounterStateComponent.mActiveEncounterType == EncounterType::TRAINER)
    {
        playerStateComponent.mDefeatedNpcEntries.emplace_back
        (
            playerStateComponent.mLastOverworldLevelName,
            playerStateComponent.mLastNpcLevelIndexSpokenTo
        );
    }
    
    encounterStateComponent.mFlowStateManager.SetActiveFlowState(nullptr);
    encounterStateComponent.mEncounterJustFinished               = false;
    encounterStateComponent.mActiveEncounterType                 = EncounterType::NONE;
    encounterStateComponent.mOverworldEncounterAnimationState    = OverworldEncounterAnimationState::NONE;
    encounterStateComponent.mOpponentPokemonRoster.clear();
    DestroyLevel(levelModelComponent.mLevelName, mWorld);
        
    while (GetActiveTextboxEntityId(mWorld) != ecs::NULL_ENTITY_ID)
    {
        DestroyActiveTextbox(mWorld);
    }    
    
    DestroyEncounterSprites(mWorld);
    
    const auto newLevelEntityId        = LoadAndCreateLevelByName(playerStateComponent.mLastOverworldLevelName, mWorld);
    auto& encounterLevelModelComponent = mWorld.GetComponent<LevelModelComponent>(newLevelEntityId);
    
    mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>().mActiveLevelNameId = encounterLevelModelComponent.mLevelName;
    
    CreatePlayerOverworldSprite
    (
        newLevelEntityId,
        playerStateComponent.mLastOverworldDirection,
        playerStateComponent.mLastOverworldLevelOccupiedCol,
        playerStateComponent.mLastOverworldLevelOccupiedRow,
        mWorld
    );
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

