//
//  EncounterStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 23/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef EncounterStateSingletonComponent_h
#define EncounterStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/GameConstants.h"
#include "../../common/flowstates/FlowStateManager.h"
#include "../../common/utils/Timer.h"
#include "../../resources/ResourceLoadingService.h"

#include <memory>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class EncounterType
{
    NONE, WILD, TRAINER
};

enum class MainMenuActionType
{
    FIGHT, POKEMON, ITEM, RUN
};

enum class OverworldEncounterAnimationState
{
    NONE, SCREEN_FLASH, ENCOUNTER_INTRO_ANIMATION, ENCOUNTER_INTRO_ANIMATION_COMPLETE
};

enum class BindOrWrapState
{
    NOT_APPLICABLE, FIRST_ROUND, CONTINUATION
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const std::unordered_map<MainMenuActionType, std::pair<int, int>> sMainMenuActionTypesToCursorCoords =
{
    { MainMenuActionType::FIGHT,   { 0, 0 } },
    { MainMenuActionType::POKEMON, { 1, 0 } },
    { MainMenuActionType::ITEM,    { 0, 1 } },
    { MainMenuActionType::RUN,     { 1, 1 } }
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const float BALL_USAGE_ANIMATION_FRAME_DURATION  = 0.03f;
static const float BATTLE_MOVE_ANIMATION_FRAME_DURATION = 0.05f;
static const float BATTLE_SPECIAL_MOVE_ANIMATION_DELAY  = 0.13f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class EncounterViewObjects final
{    
public:
    std::queue<ResourceId> mBattleAnimationFrameResourceIdQueue;    
    std::unique_ptr<Timer> mBattleAnimationTimer;
    ecs::EntityId mBattleAnimationFrameEntityId       = ecs::NULL_ENTITY_ID;            
    ecs::EntityId mPlayerActiveSpriteEntityId         = ecs::NULL_ENTITY_ID;
    ecs::EntityId mOpponentActiveSpriteEntityId       = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPlayerStatusDisplayEntityId        = ecs::NULL_ENTITY_ID;
    ecs::EntityId mOpponentStatusDisplayEntityId      = ecs::NULL_ENTITY_ID;    
    ecs::EntityId mLevelLeftEdgeEntityId              = ecs::NULL_ENTITY_ID;
    ecs::EntityId mLevelRightEdgeEntityId             = ecs::NULL_ENTITY_ID;
    ecs::EntityId mOpponentPokemonHealthBarEntityId   = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPlayerPokemonHealthBarEntityId     = ecs::NULL_ENTITY_ID;
    ecs::EntityId mFightMenuMoveInfoTexbotxEntityId   = ecs::NULL_ENTITY_ID;
    ecs::EntityId mOpponentPokemonInfoTextboxEntityId = ecs::NULL_ENTITY_ID;
    ecs::EntityId mPlayerPokemonInfoTextboxEntityId   = ecs::NULL_ENTITY_ID;
    ecs::EntityId mOpponentPokemonDeathCoverEntityId  = ecs::NULL_ENTITY_ID;
    ecs::EntityId mLevelUpNewStatsTextboxEntityId     = ecs::NULL_ENTITY_ID;
    int mBallAnimationFrameCounter                    = 0;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class EncounterStateSingletonComponent final: public ecs::IComponent
{
public:    
    FlowStateManager mFlowStateManager;
    EncounterViewObjects mViewObjects;
    std::vector<std::unique_ptr<Pokemon>> mOpponentPokemonRoster;        
    std::vector<size_t> mPlayerPokemonIndicesEligibleForXp;
    std::unordered_map<int, std::unordered_map<int, float>> mPlayerPokemonToOpponentPokemonDamageMap;
    StringId mLastMoveSelected                                           = StringId();
    StringId mOpponentTrainerSpeciesName                                 = StringId();
    StringId mOpponentTrainerName                                        = StringId();
    StringId mOpponentTrainerDefeatedText                                = StringId();
    PokemonStatus mPendingStatusToBeAppliedToPlayerPokemon               = PokemonStatus::NORMAL;    
    PokemonStatus mPendingStatusToBeAppliedToOpponentPokemon             = PokemonStatus::NORMAL;    
    EncounterType mActiveEncounterType                                   = EncounterType::NONE;
    OverworldEncounterAnimationState mOverworldEncounterAnimationState   = OverworldEncounterAnimationState::NONE;            
    MainMenuActionType mLastEncounterMainMenuActionSelected              = MainMenuActionType::FIGHT;
    BindOrWrapState mBindOrWrapState                                     = BindOrWrapState::NOT_APPLICABLE;
    size_t mNumberOfPlayerPokemonEligibleForXp                           = 0;
    int mActivePlayerPokemonRosterIndex                                  = 0;
    int mActiveOpponentPokemonRosterIndex                                = 0;
    int mSpecialMoveAnimationStep                                        = 0;
    int mLastPlayerSelectedMoveIndexFromFightMenu                        = 0;
    int mTurnsCompleted                                                  = 0;
    int mNumberOfEscapeAttempts                                          = 0;
    int mBallThrownShakeCount                                            = 0;
    float mOutstandingFloatDamage                                        = 0.0f;
    float mDefenderFloatHealth                                           = 0.0f;   
    bool mLastMoveCrit                                                   = false;
    bool mLastMoveMiss                                                   = false;
    bool mIsOpponentsTurn                                                = false;
    bool mPlayerChangedPokemonFromMainMenu                               = false;
    bool mNothingHappenedFromMoveExecution                               = false;
    bool mMoveHadNoEffect                                                = false;
    bool mHasEscapeSucceeded                                             = false;
    bool mEncounterJustFinished                                          = false;
    bool mOpponentPendingSummoning                                       = false;
    bool mAttackingPokemonIsFullyParalyzed                               = false;
    bool mPlayerDecidedToChangePokemonBeforeNewOpponentPokemonIsSummoned = false;
    bool mWasPokemonCaught                                               = false;
    bool mHasPokemonEvolvedInBattle                                      = false;    
    bool mIsGymLeaderBattle                                              = false;
    bool mIsPikachuCaptureFlowActive                                     = false;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* EncounterStateSingletonComponent_h */
