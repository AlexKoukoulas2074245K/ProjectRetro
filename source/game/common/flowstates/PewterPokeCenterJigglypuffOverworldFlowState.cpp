//
//  PewterPokeCenterJigglypuffOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 05/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PewterPokeCenterJigglypuffOverworldFlowState.h"
#include "../components/DirectionComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/NpcAiComponent.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../sound/SoundService.h"
#include "../../rendering/utils/AnimationUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PewterPokeCenterJigglypuffOverworldFlowState::JIGGLYPUFF_SONG_MUSIC_NAME = "jigglypuff_song";
const std::string PewterPokeCenterJigglypuffOverworldFlowState::LEVEL_MUSIC_NAME = "poke_center_mart";

const float PewterPokeCenterJigglypuffOverworldFlowState::DIRECTION_CHANGE_DELAY = 0.45f;

const int PewterPokeCenterJigglypuffOverworldFlowState::MAX_DIRECTION_CHANGES             = 16;
const int PewterPokeCenterJigglypuffOverworldFlowState::JIGGLYPUFF_NPC_LEVEL_ENTITY_INDEX = 4;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PewterPokeCenterJigglypuffOverworldFlowState::PewterPokeCenterJigglypuffOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mDirectionChangeTimer(DIRECTION_CHANGE_DELAY)
    , mDirectionChangeCounter(0)
{    
    SoundService::GetInstance().PlayMusic(JIGGLYPUFF_SONG_MUSIC_NAME, false);
}

void PewterPokeCenterJigglypuffOverworldFlowState::VUpdate(const float dt)
{
    mDirectionChangeTimer.Update(dt);
    if (mDirectionChangeTimer.HasTicked())
    {
        mDirectionChangeTimer.Reset();

        const auto jigglypuffEntityId = GetNpcEntityIdFromLevelIndex(JIGGLYPUFF_NPC_LEVEL_ENTITY_INDEX, mWorld);
        
        auto& jigglypuffDirectionComponent  = mWorld.GetComponent<DirectionComponent>(jigglypuffEntityId);
        auto& jigglypuffRenderableComponent = mWorld.GetComponent<RenderableComponent>(jigglypuffEntityId);
       
        jigglypuffDirectionComponent.mDirection = static_cast<Direction>((static_cast<int>(jigglypuffDirectionComponent.mDirection) + 1) % 4);
        ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(jigglypuffDirectionComponent.mDirection), jigglypuffRenderableComponent);

        if (++mDirectionChangeCounter == MAX_DIRECTION_CHANGES)
        {
            SoundService::GetInstance().PlayMusic(LEVEL_MUSIC_NAME, false);

            DestroyActiveTextbox(mWorld);
            CompleteOverworldFlow();
        }        
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


