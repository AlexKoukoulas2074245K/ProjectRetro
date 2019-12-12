//
//  PewterMuseumAerodactylFossilOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/12/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PewterMuseumAerodactylFossilOverworldFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const glm::vec3 AERODACTYL_FOSSIL_TEXTBOX_POSITION = glm::vec3(0.0f, 0.0f, 0.0f);

const int AERODACTYL_FOSSIL_TEXTBOX_COLS = 9; 
const int AERODACTYL_FOSSIL_TEXTBOX_ROWS = 10;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PewterMuseumAerodactylFossilOverworldFlowState::PewterMuseumAerodactylFossilOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mBackgroundEntityId(ecs::NULL_ENTITY_ID)
    , mPlayerIconEntityId(ecs::NULL_ENTITY_ID)
    , mCursorIconEntityId(ecs::NULL_ENTITY_ID)
    , mLocationNameTextboxEntityId(ecs::NULL_ENTITY_ID)
    , mUnknownAreaTextboxEntityId(ecs::NULL_ENTITY_ID)
    , mCursorBlinkingTimer(CURSOR_BLINKING_DELAY)
{               
    const auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
    const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();

    if (pokedexStateComponent.mSelectedPokemonName != StringId())
    {
        mDisplayMode = DisplayMode::NEST_MODE;
    }                
    else
    {
        mDisplayMode = DisplayMode::CURSOR_MODE;    
    }

    mBackgroundEntityId = LoadAndCreateTownMapBackground(mWorld);
    mPlayerIconEntityId = LoadAndCreateTownMapIconAtLocation(TownMapIconType::PLAYER_ICON, activeLevelComponent.mActiveLevelNameId, mWorld);

    if (mDisplayMode == DisplayMode::NEST_MODE)
    {
        const auto& levelNamesWherePokemonCanBeEncountered = FindAllLevelNamesWherePokemonCanBeEncountered(pokedexStateComponent.mSelectedPokemonName);
        if (levelNamesWherePokemonCanBeEncountered.size() > 0)
        {
            for (const auto& levelName : levelNamesWherePokemonCanBeEncountered)
            {
                mNestIconEntityIds.push_back(LoadAndCreateTownMapIconAtLocation(TownMapIconType::NEST_ICON, levelName, mWorld));
            }
        }
        else
        {
            mDisplayMode = DisplayMode::UNKNOWN_AREA_MODE;

            mWorld.DestroyEntity(mPlayerIconEntityId);
            mPlayerIconEntityId = ecs::NULL_ENTITY_ID;

            mUnknownAreaTextboxEntityId = CreateTextboxWithDimensions
            (
                TextboxType::GENERIC_TEXTBOX,
                UNKNOWN_AREA_TEXTBOX_COLS,
                UNKNOWN_AREA_TEXTBOX_ROWS,
                UNKNOWN_AREA_TEXTBOX_POSITION.x,
                UNKNOWN_AREA_TEXTBOX_POSITION.y,
                UNKNOWN_AREA_TEXTBOX_POSITION.z,
                mWorld
            );

            WriteTextAtTextboxCoords(mUnknownAreaTextboxEntityId, "AREA UNKNOWN", 2, 2, mWorld);
        }        
    }
    else
    {
        mCursorIconEntityId = LoadAndCreateTownMapIconAtLocation(TownMapIconType::CURSOR_ICON, activeLevelComponent.mActiveLevelNameId, mWorld);
        mCursorMapIndex = GetLocationIndexInTownMap(activeLevelComponent.mActiveLevelNameId, mWorld);
        DestroyActiveTextbox(mWorld);
    }               

    CreateLocationNameTextbox(mDisplayMode == DisplayMode::CURSOR_MODE ? GetLocationFromTownMapIndex(mCursorMapIndex, mWorld) : pokedexStateComponent.mSelectedPokemonName);
}

void TownMapOverworldFlowState::VUpdate(const float dt)
{
    mCursorBlinkingTimer.Update(dt);
    if (mCursorBlinkingTimer.HasTicked())
    {
        mCursorBlinkingTimer.Reset();

        if (mDisplayMode == DisplayMode::NEST_MODE)
        {
            for (const auto& nestIconEntityId : mNestIconEntityIds)
            {
                auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(nestIconEntityId);
                renderableComponent.mVisibility = !renderableComponent.mVisibility;
            }
        }
        else if (mDisplayMode == DisplayMode::CURSOR_MODE)
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
        
        if (mUnknownAreaTextboxEntityId != ecs::NULL_ENTITY_ID)
        {
            DestroyGenericOrBareTextbox(mUnknownAreaTextboxEntityId, mWorld);
        }

        for (const auto nestIconEntityId : mNestIconEntityIds)
        {
            mWorld.DestroyEntity(nestIconEntityId);
        }

        DestroyGenericOrBareTextbox(mLocationNameTextboxEntityId, mWorld);

        mWorld.DestroyEntity(mBackgroundEntityId);        

        if (mDisplayMode == DisplayMode::CURSOR_MODE)
        {
            CompleteOverworldFlow();
        }
        else
        {
            CompleteAndTransitionTo<PokedexMainViewOverworldFlowState>();
        }        
    }    
    else if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent) && mDisplayMode == DisplayMode::CURSOR_MODE)
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
    else if (IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent) && mDisplayMode == DisplayMode::CURSOR_MODE)
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

void PewterMuseumAerodactylFossilOverworldFlowState::CreateLocationNameTextbox(const StringId name)
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
    
    if (mDisplayMode == DisplayMode::CURSOR_MODE)
    {
        WriteTextAtTextboxCoords(mLocationNameTextboxEntityId, GetFormattedLocationName(name), 0, 0, mWorld);
    }
    else
    {
        WriteTextAtTextboxCoords(mLocationNameTextboxEntityId, name.GetString() + "'s NEST", 0, 0, mWorld);
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
