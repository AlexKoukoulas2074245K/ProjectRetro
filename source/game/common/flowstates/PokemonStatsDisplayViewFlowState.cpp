//
//  PokemonStatsDisplayViewFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 23/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonStatsDisplayViewFlowState.h"
#include "PokemonSelectionViewFlowState.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../components/PokemonStatsDisplayViewStateSingletonComponent.h"
#include "../components/TextboxComponent.h"
#include "../utils/PokemonSelectionViewSpriteUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PokemonStatsDisplayViewFlowState::BACKGROUND_POSITION     = glm::vec3(0.0f, 0.0f, 0.01f);
const glm::vec3 PokemonStatsDisplayViewFlowState::BACKGROUND_SCALE        = glm::vec3(2.0f, 2.0f, 2.0f);
const glm::vec3 PokemonStatsDisplayViewFlowState::POKEMON_SPRITE_POSITION = glm::vec3(-0.37f, 0.61f, -0.8f);
const glm::vec3 PokemonStatsDisplayViewFlowState::POKEMON_SPRITE_SCALE    = glm::vec3(-0.49f, 0.49f, 1.0f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokemonStatsDisplayViewFlowState::PokemonStatsDisplayViewFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& pokemonStatsDisplayViewStateComponent        = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();
    pokemonStatsDisplayViewStateComponent.mIsInScreen1 = true;    

    CreatePokemonStatsBackground();
    LoadAndCreatePokemonStatsScreen1();
}

void PokemonStatsDisplayViewFlowState::VUpdate(const float)
{        
    const auto& inputStateComponent             = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& pokemonStatsDisplayViewStateComponent = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();

    if 
    (
        IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||
        IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent)
    ) 
    {
        if (pokemonStatsDisplayViewStateComponent.mIsInScreen1)
        {
            pokemonStatsDisplayViewStateComponent.mIsInScreen1 = false;
            DestroyPokemonStatsScreen();
            LoadAndCreatePokemonStatsScreen2();
        }
        else
        {
            DestroyPokemonStatsScreen();
            DestroyPokemonStatsBackground();
            CompleteAndTransitionTo<PokemonSelectionViewFlowState>();            
        }
    } 
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PokemonStatsDisplayViewFlowState::CreatePokemonStatsBackground() const
{
    auto& pokemonStatsDisplayViewStateComponent = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();
    pokemonStatsDisplayViewStateComponent.mBackgroundCoverEntityId = LoadAndCreateBackgroundCover
    (
        BACKGROUND_POSITION, 
        BACKGROUND_SCALE, mWorld
    );
}

void PokemonStatsDisplayViewFlowState::LoadAndCreatePokemonStatsScreen1() const
{
    const auto& playerStateComponent               = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& pokemonSelectionViewStateComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    const auto& selectedPokemon                    = *playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex];
    auto& pokemonStatsDisplayViewStateComponent    = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();

    pokemonStatsDisplayViewStateComponent.mStatsLayoutsEntityId = LoadAndCreatePokemonStatsDisplayScreen(true, mWorld);    

    pokemonStatsDisplayViewStateComponent.mPokemonFrontSpriteEntityId = LoadAndCreatePokemonSprite
    (
        selectedPokemon.mBaseSpeciesStats.mSpeciesName.GetString(),
        true,
        POKEMON_SPRITE_POSITION,
        POKEMON_SPRITE_SCALE,
        mWorld
    );

    
    pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId = CreatePokemonStatsDisplayTextbox(mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, GetFormattedPokemonIdString(selectedPokemon.mBaseSpeciesStats.mId), 3, 7, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, GetFormattedPokemonStatus(selectedPokemon.mHp, selectedPokemon.mStatus), 16, 6, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, selectedPokemon.mName.GetString(), 9, 1, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, "=" + std::to_string(selectedPokemon.mLevel), 14, 2, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, std::to_string(selectedPokemon.mAttack), 9 - std::to_string(selectedPokemon.mAttack).size(), 10, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, std::to_string(selectedPokemon.mDefense), 9 - std::to_string(selectedPokemon.mDefense).size(), 12, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, std::to_string(selectedPokemon.mSpeed), 9 - std::to_string(selectedPokemon.mSpeed).size(), 14, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, std::to_string(selectedPokemon.mSpecial), 9 - std::to_string(selectedPokemon.mSpecial).size(), 16, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, std::to_string(playerStateComponent.mTrainerId), 12, 14, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, playerStateComponent.mTrainerName.GetString(), 12, 16, mWorld);

    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, "TYPE1/", 10, 9, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, selectedPokemon.mBaseSpeciesStats.mFirstType.GetString(), 11, 10, mWorld);

    if (selectedPokemon.mBaseSpeciesStats.mSecondType != StringId())
    {
        WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, "TYPE2/", 10, 11, mWorld);
        WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, selectedPokemon.mBaseSpeciesStats.mSecondType.GetString(), 11, 12, mWorld);
    }
}

void PokemonStatsDisplayViewFlowState::LoadAndCreatePokemonStatsScreen2() const
{    
    const auto& playerStateComponent               = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& pokemonSelectionViewStateComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    const auto& selectedPokemon                    = *playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex];
    auto& pokemonStatsDisplayViewStateComponent    = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();

    pokemonStatsDisplayViewStateComponent.mStatsLayoutsEntityId = LoadAndCreatePokemonStatsDisplayScreen(false, mWorld);    

    pokemonStatsDisplayViewStateComponent.mPokemonFrontSpriteEntityId = LoadAndCreatePokemonSprite
    (
        selectedPokemon.mBaseSpeciesStats.mSpeciesName.GetString(),
        true,
        POKEMON_SPRITE_POSITION,
        POKEMON_SPRITE_SCALE,
        mWorld
    );

    pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId = CreatePokemonStatsDisplayTextbox(mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, GetFormattedPokemonIdString(selectedPokemon.mBaseSpeciesStats.mId), 3, 7, mWorld);    
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, selectedPokemon.mName.GetString(), 9, 1, mWorld);
    
}

void PokemonStatsDisplayViewFlowState::DestroyPokemonStatsScreen() const
{
    auto& pokemonStatsDisplayViewStateComponent = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();    
    mWorld.RemoveEntity(pokemonStatsDisplayViewStateComponent.mStatsLayoutsEntityId);
    mWorld.RemoveEntity(pokemonStatsDisplayViewStateComponent.mPokemonFrontSpriteEntityId);
    pokemonStatsDisplayViewStateComponent.mStatsLayoutsEntityId = ecs::NULL_ENTITY_ID;
    pokemonStatsDisplayViewStateComponent.mPokemonFrontSpriteEntityId = ecs::NULL_ENTITY_ID;
    DestroyGenericOrBareTextbox(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, mWorld);
}

void PokemonStatsDisplayViewFlowState::DestroyPokemonStatsBackground() const
{
    auto& pokemonStatsDisplayViewStateComponent = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();
    mWorld.RemoveEntity(pokemonStatsDisplayViewStateComponent.mBackgroundCoverEntityId);
    pokemonStatsDisplayViewStateComponent.mBackgroundCoverEntityId = ecs::NULL_ENTITY_ID;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
