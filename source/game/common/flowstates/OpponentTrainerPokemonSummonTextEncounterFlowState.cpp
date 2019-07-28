//
//  OpponentTrainerPokemonSummonTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 28/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OpponentTrainerPokemonSummonTextEncounterFlowState.h"
#include "OpponentPokemonStatusDisplayEncounterFlowState.h"
#include "../../common/components/GuiStateSingletonComponent.h"
#include "../../common/utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 OpponentTrainerPokemonSummonTextEncounterFlowState::OPPONENT_SPRITE_TARGET_POS = glm::vec3(0.38f, 0.61f, 0.3f);
const glm::vec3 OpponentTrainerPokemonSummonTextEncounterFlowState::SPRITE_SCALE               = glm::vec3(0.49f, 0.49f, 1.0f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OpponentTrainerPokemonSummonTextEncounterFlowState::OpponentTrainerPokemonSummonTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& activeOpponentPokemon   = *encounterStateComponent.mOpponentPokemonRoster.at(encounterStateComponent.mActiveOpponentPokemonRosterIndex);
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        encounterStateComponent.mOpponentTrainerName.GetString() + " sent#out " + activeOpponentPokemon.mName.GetString() + "!+FREEZE",
        mWorld
    );
}

void OpponentTrainerPokemonSummonTextEncounterFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    
    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {
        DestroyActiveTextbox(mWorld);
        
        mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
        
        encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId = LoadAndCreatePokemonSprite
        (
            encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex]->mBaseSpeciesStats.mSpeciesName,
            true,
            OPPONENT_SPRITE_TARGET_POS,
            SPRITE_SCALE,
            mWorld
        );
        
        CompleteAndTransitionTo<OpponentPokemonStatusDisplayEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

