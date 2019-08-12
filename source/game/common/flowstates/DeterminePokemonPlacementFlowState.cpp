//
//  DeterminePokemonPlacementFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "DeterminePokemonPlacementFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

DeterminePokemonPlacementFlowState::DeterminePokemonPlacementFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    if (playerStateComponent.mPlayerPokemonRoster.size() < MAX_ROSTER_SIZE)
    {
        playerStateComponent.mPlayerPokemonRoster.push_back(std::move(encounterStateComponent.mOpponentPokemonRoster.front()));
    }
    else
    {
        const auto mainChatboxEntityId = CreateChatbox(mWorld);
        
        std::string chatboxText = encounterStateComponent.mOpponentPokemonRoster.front()->mName.GetString() + " was#transferred to#";
        QueueDialogForChatbox
        (
            mainChatboxEntityId,
            playerStateComponent.mBillInteractionCompleted ? "BILL's PC!# #+END" : "someone's PC!# #+END",
            mWorld
        );

        //TODO: add pokemon to box
    }
}

void DeterminePokemonPlacementFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        DestroyRemainingEncounterSpritesAndContinueToOverworld();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void DeterminePokemonPlacementFlowState::DestroyRemainingEncounterSpritesAndContinueToOverworld() const
{
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    // Destroy Last Frame of Pokemon Caught animation
    if (encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId != ecs::NULL_ENTITY_ID)
    {
        mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId);
        encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = ecs::NULL_ENTITY_ID;
    }

    // Destroy Opponent pokemon sprite
    if (encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId != ecs::NULL_ENTITY_ID)
    {
        mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
        encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId = ecs::NULL_ENTITY_ID;
    }

    encounterStateComponent.mEncounterJustFinished = true;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
