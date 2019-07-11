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
        
        const auto damage = CalculateDamage(activePlayerPokemon.mLevel, selectedMove.mPower, activePlayerPokemon.mSpecial, activeOpponentPokemon.mSpecial, 1.0, true);
        (void)damage;
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

