//
//  PokemonStatsDisplayViewFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 23/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PCPokemonSystemDialogOverworldFlowState.h"
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
#include "../../overworld/components/PCStateSingletonComponent.h"
#include "../../sound/SoundService.h"

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
    if (SoundService::GetInstance().IsPlayingSfx()) return;

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

            switch (pokemonStatsDisplayViewStateComponent.mSourceCreatorFlow)
            {
                case PokemonStatsDisplayViewCreationSourceType::POKEMON_SELECTION_VIEW: CompleteAndTransitionTo<PokemonSelectionViewFlowState>(); break;
                case PokemonStatsDisplayViewCreationSourceType::PC:                     CompleteAndTransitionTo<PCPokemonSystemDialogOverworldFlowState>(); break;
            }            
        }
    } 
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PokemonStatsDisplayViewFlowState::CreatePokemonStatsBackground() const
{
    auto& pokemonStatsDisplayViewStateComponent = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();
    pokemonStatsDisplayViewStateComponent.mBackgroundCoverEntityId = LoadAndCreatePokemonSelectionViewBackgroundCover
    (
        BACKGROUND_POSITION, 
        BACKGROUND_SCALE, 
        mWorld
    );
}

void PokemonStatsDisplayViewFlowState::LoadAndCreatePokemonStatsScreen1() const
{   
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& selectedPokemon = GetSelectedPokemon();

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
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, playerStateComponent.mPlayerTrainerName.GetString(), 12, 16, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, std::to_string(selectedPokemon.mMaxHp), 19 - std::to_string(selectedPokemon.mMaxHp).size(), 4, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, std::to_string(selectedPokemon.mHp), 15 - std::to_string(selectedPokemon.mHp).size(), 4, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, "/", 15, 4, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, "TYPE1/", 10, 9, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, selectedPokemon.mBaseSpeciesStats.mFirstType.GetString(), 11, 10, mWorld);

    if (selectedPokemon.mBaseSpeciesStats.mSecondType != StringId())
    {
        WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, "TYPE2/", 10, 11, mWorld);
        WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, selectedPokemon.mBaseSpeciesStats.mSecondType.GetString(), 11, 12, mWorld);
    }

    pokemonStatsDisplayViewStateComponent.mPokemonHealthbarEntityId = LoadAndCreatePokemonHealthBar
    (
        static_cast<float>(selectedPokemon.mHp) / selectedPokemon.mMaxHp,
        false,
        mWorld,
        true
    );

    SoundService::GetInstance().PlaySfx("cries/" + GetFormattedPokemonIdString(selectedPokemon.mBaseSpeciesStats.mId));
}

void PokemonStatsDisplayViewFlowState::LoadAndCreatePokemonStatsScreen2() const
{    
    const auto& selectedPokemon = GetSelectedPokemon();

    auto& pokemonStatsDisplayViewStateComponent = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();

    pokemonStatsDisplayViewStateComponent.mStatsLayoutsEntityId = LoadAndCreatePokemonStatsDisplayScreen(false, mWorld);    

    pokemonStatsDisplayViewStateComponent.mPokemonFrontSpriteEntityId = LoadAndCreatePokemonSprite
    (
        selectedPokemon.mBaseSpeciesStats.mSpeciesName.GetString(),
        true,
        POKEMON_SPRITE_POSITION,
        POKEMON_SPRITE_SCALE,
        mWorld
    );

    const auto xpToNextLevel = CalculatePokemonTotalExperienceAtLevel(selectedPokemon.mBaseSpeciesStats.mSpeciesName, selectedPokemon.mLevel + 1, mWorld) - selectedPokemon.mXpPoints;

    pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId = CreatePokemonStatsDisplayTextbox(mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, GetFormattedPokemonIdString(selectedPokemon.mBaseSpeciesStats.mId), 3, 7, mWorld);    
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, selectedPokemon.mBaseSpeciesStats.mSpeciesName.GetString(), 9, 1, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, std::to_string(selectedPokemon.mXpPoints), 19 - std::to_string(selectedPokemon.mXpPoints).size(), 4, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, std::to_string(xpToNextLevel), 14 - std::to_string(xpToNextLevel).size(), 6, mWorld);
    WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, "=" + std::to_string(selectedPokemon.mLevel + 1), 18 - std::to_string(selectedPokemon.mLevel + 1).size(), 6, mWorld);

    for (auto i = 0U; i < selectedPokemon.mMoveSet.size(); ++i)
    {
        if (selectedPokemon.mMoveSet[i] != nullptr)
        {
            const auto& move = *selectedPokemon.mMoveSet[i];
            
            WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, move.mName.GetString(), 2, 9 + i * 2, mWorld);
            WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, "\\\\", 11, 10 + i * 2, mWorld);
            WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, "/", 16, 10 + i * 2, mWorld);
            WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, std::to_string(move.mPowerPointsLeft), 16 - std::to_string(move.mPowerPointsLeft).size(), 10 + i * 2, mWorld);
            WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, std::to_string(move.mTotalPowerPoints), 19 - std::to_string(move.mTotalPowerPoints).size(), 10 + i * 2, mWorld);
        }
        else
        {
            WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, "-", 2, 9 + i * 2, mWorld);
            WriteTextAtTextboxCoords(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, "--", 11, 10 + i * 2, mWorld);
        }
    }
}

void PokemonStatsDisplayViewFlowState::DestroyPokemonStatsScreen() const
{
    auto& pokemonStatsDisplayViewStateComponent = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();    

    mWorld.DestroyEntity(pokemonStatsDisplayViewStateComponent.mStatsLayoutsEntityId);
    mWorld.DestroyEntity(pokemonStatsDisplayViewStateComponent.mPokemonFrontSpriteEntityId);

    pokemonStatsDisplayViewStateComponent.mStatsLayoutsEntityId       = ecs::NULL_ENTITY_ID;
    pokemonStatsDisplayViewStateComponent.mPokemonFrontSpriteEntityId = ecs::NULL_ENTITY_ID;

    DestroyGenericOrBareTextbox(pokemonStatsDisplayViewStateComponent.mPokemonStatsInvisibleTextboxEntityId, mWorld);

    if (pokemonStatsDisplayViewStateComponent.mPokemonHealthbarEntityId != ecs::NULL_ENTITY_ID)
    {
        mWorld.DestroyEntity(pokemonStatsDisplayViewStateComponent.mPokemonHealthbarEntityId);
        pokemonStatsDisplayViewStateComponent.mPokemonHealthbarEntityId = ecs::NULL_ENTITY_ID;
    }
    
}

void PokemonStatsDisplayViewFlowState::DestroyPokemonStatsBackground() const
{
    auto& pokemonStatsDisplayViewStateComponent = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();
    mWorld.DestroyEntity(pokemonStatsDisplayViewStateComponent.mBackgroundCoverEntityId);
    pokemonStatsDisplayViewStateComponent.mBackgroundCoverEntityId = ecs::NULL_ENTITY_ID;
}

const Pokemon& PokemonStatsDisplayViewFlowState::GetSelectedPokemon() const
{
    const auto& pokemonStatsDisplayViewStateComponent = mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>();
    const auto& pokemonSelectionViewStateComponent    = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    const auto& pcSystemStateComponent                = mWorld.GetSingletonComponent<PCStateSingletonComponent>();
    const auto& playerStateComponent                  = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    switch (pokemonStatsDisplayViewStateComponent.mSourceCreatorFlow)
    {
        case PokemonStatsDisplayViewCreationSourceType::PC: 
        {
            if (pcSystemStateComponent.mPokemonSystemOperationType == PokemonSystemOperationType::DEPOSIT)
            {
                return *playerStateComponent.mPlayerPokemonRoster.at(pcSystemStateComponent.mLastSelectedPokemonIndex);
            }
            else
            {
                return *playerStateComponent.mPlayerBoxedPokemon.at(pcSystemStateComponent.mLastSelectedPokemonIndex);
            }
        }

        case PokemonStatsDisplayViewCreationSourceType::POKEMON_SELECTION_VIEW:
        {
            return *playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex];
        }
    }

    return *playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex];
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
