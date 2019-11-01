//
//  TownMapOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 24/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokedexMainViewOverworldFlowState.h"
#include "TownMapOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PokedexStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../overworld/utils/EncounterUtils.h"
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
    , mCursorIconEntityId(ecs::NULL_ENTITY_ID)
    , mLocationNameTextboxEntityId(ecs::NULL_ENTITY_ID)
    , mCursorBlinkingTimer(CURSOR_BLINKING_DELAY)
{               
    auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
    mNestDisplayMode = pokedexStateComponent.mSelectedPokemonName != StringId();
    
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
    
    mBackgroundEntityId = LoadAndCreateTownMapBackground(mWorld);    
    mPlayerIconEntityId = LoadAndCreateTownMapIconAtLocation(TownMapIconType::PLAYER_ICON, activeLevelComponent.mActiveLevelNameId, mWorld);

    if (mNestDisplayMode)
    {
        const auto& levelNamesWherePokemonCanBeEncountered = FindAllLevelNamesWherePokemonCanBeEncountered(pokedexStateComponent.mSelectedPokemonName);
        for (const auto& levelName : levelNamesWherePokemonCanBeEncountered)
        {
            mNestIconEntityIds.push_back(LoadAndCreateTownMapIconAtLocation(TownMapIconType::NEST_ICON, levelName, mWorld));
        }
    }
    else
    {
        mCursorIconEntityId = LoadAndCreateTownMapIconAtLocation(TownMapIconType::CURSOR_ICON, activeLevelComponent.mActiveLevelNameId, mWorld);
        mCursorMapIndex = GetLocationIndexInTownMap(activeLevelComponent.mActiveLevelNameId, mWorld);
        DestroyActiveTextbox(mWorld);
    }               

    CreateLocationNameTextbox(mNestDisplayMode ? pokedexStateComponent.mSelectedPokemonName : GetLocationFromTownMapIndex(mCursorMapIndex, mWorld));
}

void TownMapOverworldFlowState::VUpdate(const float dt)
{
    mCursorBlinkingTimer.Update(dt);
    if (mCursorBlinkingTimer.HasTicked())
    {
        mCursorBlinkingTimer.Reset();

        if (mNestDisplayMode)
        {
            for (const auto& nestIconEntityId : mNestIconEntityIds)
            {
                auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(nestIconEntityId);
                renderableComponent.mVisibility = !renderableComponent.mVisibility;
            }
        }
        else
        {
            auto& renderableComponent       = mWorld.GetComponent<RenderableComponent>(mCursorIconEntityId);
            renderableComponent.mVisibility = !renderableComponent.mVisibility;
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
        
        for (const auto nestIconEntityId : mNestIconEntityIds)
        {
            mWorld.DestroyEntity(nestIconEntityId);
        }

        DestroyGenericOrBareTextbox(mLocationNameTextboxEntityId, mWorld);
        mWorld.DestroyEntity(mBackgroundEntityId);        

        if (mNestDisplayMode)
        {
            CompleteAndTransitionTo<PokedexMainViewOverworldFlowState>();
        }
        else
        {
            CompleteOverworldFlow();
        }        
    }    
    else if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent) && mNestDisplayMode == false)
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
    else if (IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent) && mNestDisplayMode == false)
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

void TownMapOverworldFlowState::CreateLocationNameTextbox(const StringId name)
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
    
    if (mNestDisplayMode)
    {
        WriteTextAtTextboxCoords(mLocationNameTextboxEntityId, name.GetString() + "'s NEST", 0, 0, mWorld);
    }
    else
    {
        WriteTextAtTextboxCoords(mLocationNameTextboxEntityId, GetFormattedLocationName(name), 0, 0, mWorld);
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
