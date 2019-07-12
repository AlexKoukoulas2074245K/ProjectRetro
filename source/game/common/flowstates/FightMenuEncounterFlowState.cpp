//
//  FightMenuEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 09/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "FightMenuEncounterFlowState.h"
#include "MainMenuEncounterFlowState.h"
#include "PlayerMoveAnnouncementEncounterFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../../common/utils/Logging.h"
#include "../../common/utils/PokemonMoveUtils.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

FightMenuEncounterFlowState::FightMenuEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    const auto& playerPokemonMoveset = playerStateComponent.mPlayerPokemonRoster.front()->mMoveSet;
    CreateEncounterFightMenuTextbox(playerPokemonMoveset, encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu, mWorld);
    encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId = CreateEncounterFightMenuMoveInfoTextbox(*playerPokemonMoveset[encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu], mWorld);
}

void FightMenuEncounterFlowState::VUpdate(const float)
{
    const auto& cursorComponent     = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();    
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (IsActionTypeKeyTapped(VirtualActionType::A, inputStateComponent))
    {
        //TODO: Here probably check move pp
        encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu = cursorComponent.mCursorRow;

        // Destroy fight menu textbox
        DestroyActiveTextbox(mWorld);

        // Destroy move info textbox
        DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId, mWorld);

        // Calculate Damage        
        auto& playerStateComponent        = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
        const auto& activePlayerPokemon   = *playerStateComponent.mPlayerPokemonRoster.front();
        const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster.front();
        const auto& selectedMove          = *activePlayerPokemon.mMoveSet[encounterStateComponent.mLastPlayerSelectedMoveIndexFromFightMenu];

        encounterStateComponent.mLastMoveMiss = false;
        encounterStateComponent.mLastMoveCrit = false;

        encounterStateComponent.mOutstandingFloatDamage = 0.0f;
        encounterStateComponent.mDefenderFloatHealth    = static_cast<float>(activeOpponentPokemon.mHp);

        if (ShouldMoveMiss(selectedMove.mAccuracy, activePlayerPokemon.mAccuracyStage, activeOpponentPokemon.mEvasionStage))
        {
            encounterStateComponent.mLastMoveMiss = true;
        }
        else
        {                
            const auto isCrit = ShouldMoveCrit(selectedMove.mName, activePlayerPokemon.mSpeed);       
            auto isStab       = selectedMove.mType == activePlayerPokemon.mBaseStats.mFirstType || selectedMove.mType == activePlayerPokemon.mBaseStats.mSecondType;            

            auto effectivenessFactor = GetTypeEffectiveness(selectedMove.mType, activeOpponentPokemon.mBaseStats.mFirstType, mWorld);
            if (activeOpponentPokemon.mBaseStats.mSecondType != StringId())
            {
                effectivenessFactor *= GetTypeEffectiveness(selectedMove.mType, activeOpponentPokemon.mBaseStats.mSecondType, mWorld);
            }

            encounterStateComponent.mOutstandingFloatDamage = static_cast<float>(CalculateDamage
            (
                activePlayerPokemon.mLevel,
                selectedMove.mPower,
                activePlayerPokemon.mSpecial, 
                activeOpponentPokemon.mSpecial, 
                effectivenessFactor, 
                isCrit, 
                isStab
            ));
        }                                    

        CompleteAndTransitionTo<PlayerMoveAnnouncementEncounterFlowState>();
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B, inputStateComponent))
    {
        // Destroy fight menu textbox
        DestroyActiveTextbox(mWorld);

        // Destroy move info textbox
        DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId, mWorld);

        CompleteAndTransitionTo<MainMenuEncounterFlowState>();
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::UP, inputStateComponent) || IsActionTypeKeyTapped(VirtualActionType::DOWN, inputStateComponent))
    {        
        // Destroy move info textbox
        DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId, mWorld);

        auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
        const auto& playerPokemonMoveset = playerStateComponent.mPlayerPokemonRoster.front()->mMoveSet;
        encounterStateComponent.mViewObjects.mFightMenuMoveInfoTexbotxEntityId = CreateEncounterFightMenuMoveInfoTextbox(*playerPokemonMoveset[cursorComponent.mCursorRow], mWorld);
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

