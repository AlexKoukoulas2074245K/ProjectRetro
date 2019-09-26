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
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/utils/TownMapUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 TownMapOverworldFlowState::LOCATION_NAME_TEXTBOX_POSITION = glm::vec3(-0.0633000061f, 0.927501202f, -0.5f);

const std::string TownMapOverworldFlowState::CURSOR_BUMP_SFX_NAME = "general/pokeball_healing";

const float TownMapOverworldFlowState::CURSOR_BLINKING_DELAY = 0.5f;

const int TownMapOverworldFlowState::LOCATION_NAME_TEXTBOX_COLS = 16;
const int TownMapOverworldFlowState::LOCATION_NAME_TEXTBOX_ROWS = 1;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TownMapOverworldFlowState::TownMapOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mLocationNameTextboxEntityId(ecs::NULL_ENTITY_ID)
    , mCursorBlinkingTimer(CURSOR_BLINKING_DELAY)
{   
    DestroyActiveTextbox(mWorld);

    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();

    mBackgroundEntityId = LoadAndCreateTownMapBackground(mWorld);    
    mPlayerIconEntityId = LoadAndCreateTownMapIconAtLocation(TownMapIconType::PLAYER_ICON, activeLevelComponent.mActiveLevelNameId, mWorld);
    mCursorIconEntityId = LoadAndCreateTownMapIconAtLocation(TownMapIconType::CURSOR_ICON, activeLevelComponent.mActiveLevelNameId, mWorld);
    mCursorMapIndex     = GetLocationIndexInTownMap(activeLevelComponent.mActiveLevelNameId, mWorld);
    
    CreateLocationNameTextbox(GetLocationFromTownMapIndex(mCursorMapIndex, mWorld));
}

void TownMapOverworldFlowState::VUpdate(const float dt)
{
    mCursorBlinkingTimer.Update(dt);
    if (mCursorBlinkingTimer.HasTicked())
    {
        mCursorBlinkingTimer.Reset();

        if (mCursorIconEntityId != ecs::NULL_ENTITY_ID)
        {
            mWorld.DestroyEntity(mCursorIconEntityId);
            mCursorIconEntityId = ecs::NULL_ENTITY_ID;
        }
        else
        {
            mCursorIconEntityId = LoadAndCreateTownMapIconAtLocation(TownMapIconType::CURSOR_ICON, GetLocationFromTownMapIndex(mCursorMapIndex, mWorld), mWorld);
        }
    }

    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();

    if 
    (
        IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||
        IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent)
    )
    {        
        if (mCursorIconEntityId != ecs::NULL_ENTITY_ID)
        {
            mWorld.DestroyEntity(mCursorIconEntityId);
        }
        
        if (mPlayerIconEntityId != ecs::NULL_ENTITY_ID)
        {
            mWorld.DestroyEntity(mPlayerIconEntityId);
        }
        
        DestroyGenericOrBareTextbox(mLocationNameTextboxEntityId, mWorld);
        mWorld.DestroyEntity(mBackgroundEntityId);        
        CompleteOverworldFlow();
    }    
    else if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent))
    {
        SoundService::GetInstance().PlaySfx(CURSOR_BUMP_SFX_NAME, true);

        if (mCursorIconEntityId != ecs::NULL_ENTITY_ID)
        {
            mWorld.DestroyEntity(mCursorIconEntityId);
        }

        if (++mCursorMapIndex >= GetTownMapLocationCount(mWorld))
        {
            mCursorMapIndex = 0;
        }
        
        const auto newLocation = GetLocationFromTownMapIndex(mCursorMapIndex, mWorld);
        mCursorIconEntityId = LoadAndCreateTownMapIconAtLocation(TownMapIconType::CURSOR_ICON, newLocation, mWorld);        
        CreateLocationNameTextbox(newLocation);
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent))
    {
        SoundService::GetInstance().PlaySfx(CURSOR_BUMP_SFX_NAME, true);

        if (mCursorIconEntityId != ecs::NULL_ENTITY_ID)
        {
            mWorld.DestroyEntity(mCursorIconEntityId);
        }

        if (--mCursorMapIndex < 0)
        {
            mCursorMapIndex = GetTownMapLocationCount(mWorld) - 1;
        }

        const auto newLocation = GetLocationFromTownMapIndex(mCursorMapIndex, mWorld);
        mCursorIconEntityId = LoadAndCreateTownMapIconAtLocation(TownMapIconType::CURSOR_ICON, newLocation, mWorld);
        CreateLocationNameTextbox(newLocation);
    }        
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void TownMapOverworldFlowState::CreateLocationNameTextbox(const StringId locationName)
{
    if (mLocationNameTextboxEntityId != ecs::NULL_ENTITY_ID)
    {
        DestroyGenericOrBareTextbox(mLocationNameTextboxEntityId, mWorld);
    }

    mLocationNameTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::BARE_TEXTBOX,
        LOCATION_NAME_TEXTBOX_COLS,
        LOCATION_NAME_TEXTBOX_ROWS,
        LOCATION_NAME_TEXTBOX_POSITION.x,
        LOCATION_NAME_TEXTBOX_POSITION.y,
        LOCATION_NAME_TEXTBOX_POSITION.z,
        mWorld
    );
    
    WriteTextAtTextboxCoords(mLocationNameTextboxEntityId, GetFormattedLocationName(locationName), 0, 0, mWorld);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
