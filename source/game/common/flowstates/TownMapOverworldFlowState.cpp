//
//  TownMapOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 24/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TownMapOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/utils/TownMapUtils.h"
#include "../components/TransformComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float TownMapOverworldFlowState::CURSOR_BLINKING_DELAY = 0.5f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TownMapOverworldFlowState::TownMapOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mCursorBlinkingTimer(CURSOR_BLINKING_DELAY)        
{                
    //const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();

    mBackgroundEntityId = LoadAndCreateTownMapBackground(mWorld);    
    mPlayerIconEntityId = LoadAndCreateTownMapIconAtLocation(TownMapIconType::CURSOR_ICON, StringId("pewter_city"), mWorld);

    DestroyActiveTextbox(mWorld);
}

void TownMapOverworldFlowState::VUpdate(const float dt)
{
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();

    if 
    (
        IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||
        IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent)
    )
    {
        mWorld.DestroyEntity(mBackgroundEntityId);        
        CompleteOverworldFlow();
    }
    else if (IsActionTypeKeyPressed(VirtualActionType::LEFT_ARROW, inputStateComponent))
    {
        auto& transformComponent = mWorld.GetComponent<TransformComponent>(mPlayerIconEntityId);
        transformComponent.mPosition.x -= 0.1f * dt;
    }
    else if (IsActionTypeKeyPressed(VirtualActionType::RIGHT_ARROW, inputStateComponent))
    {
        auto& transformComponent = mWorld.GetComponent<TransformComponent>(mPlayerIconEntityId);
        transformComponent.mPosition.x += 0.1f * dt;
    }
    else if (IsActionTypeKeyPressed(VirtualActionType::UP_ARROW, inputStateComponent))
    {
        auto& transformComponent = mWorld.GetComponent<TransformComponent>(mPlayerIconEntityId);
        transformComponent.mPosition.y += 0.1f * dt;
    }
    else if (IsActionTypeKeyPressed(VirtualActionType::DOWN_ARROW, inputStateComponent))
    {
        auto& transformComponent = mWorld.GetComponent<TransformComponent>(mPlayerIconEntityId);
        transformComponent.mPosition.y -= 0.1f * dt;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


