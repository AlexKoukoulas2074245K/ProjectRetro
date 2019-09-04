//
//  PokedexPokemonEntryDisplayFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokedexPokemonEntryDisplayFlowState.h"
#include "PokemonNicknameQuestionTextEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PokedexStateSingletonComponent.h"
#include "../utils/PokedexSpriteUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PokedexPokemonEntryDisplayFlowState::POKEMON_SPRITE_POSITION = glm::vec3(-0.4f, 0.5f, -0.8f);
const glm::vec3 PokedexPokemonEntryDisplayFlowState::POKEMON_SPRITE_SCALE    = glm::vec3(-0.49f, 0.49f, 1.0f);

const float PokedexPokemonEntryDisplayFlowState::CHATBOX_BLINKING_CURSOR_COOLDOWN = 0.7f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokedexPokemonEntryDisplayFlowState::PokedexPokemonEntryDisplayFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
    pokedexStateComponent.mPokedexBackgroundSpriteEntityId = LoadAndCreatePokedexPokemonDataScreen(mWorld);
    pokedexStateComponent.mPokedexViewTimer                = std::make_unique<Timer>(POKEDEX_VIEW_TIMER_DURATION);
    pokedexStateComponent.mCurrentPageViewType             = PokedexPageEntryType::LOCKED;
    DisplayLockedStatsText();
}

void PokedexPokemonEntryDisplayFlowState::VUpdate(const float dt)
{          
    if (SoundService::GetInstance().IsPlayingSfx()) return;

    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& pokedexStateComponent     = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();

    switch (pokedexStateComponent.mCurrentPageViewType)
    {
        case PokedexPageEntryType::LOCKED:
        {
            pokedexStateComponent.mPokedexViewTimer->Update(dt);
            if (pokedexStateComponent.mPokedexViewTimer->HasTicked())
            {                
                if (GetPokedexEntryTypeForPokemon(pokedexStateComponent.mSelectedPokemonName, mWorld) != PokedexEntryType::OWNED)
                {
                    pokedexStateComponent.mCurrentPageViewType = PokedexPageEntryType::DETAILS_LOCKED;
                }                
                else
                {
                    DisplayPokemonBodyStatsText();
                    DisplayPokemonPokedexDescriptionForPage(0);

                    pokedexStateComponent.mCurrentPageViewType                    = PokedexPageEntryType::DETAILS_1;
                    pokedexStateComponent.mPokedexViewTimer                       = std::make_unique<Timer>(CHATBOX_BLINKING_CURSOR_COOLDOWN);
                    pokedexStateComponent.mPokedexPokemonDescriptionCursorShowing = false;
                }

                DisplayPokemonSprite();
                
                pokedexStateComponent.mPokedexViewTimer->Reset();
            }
        } break;
        
        case PokedexPageEntryType::DETAILS_LOCKED:
        {            
            if
            (
                IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||
                IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent)
            )
            {
                //TODO: CompleteAndTransitionTo<PokedexViewFlowState>();
            }
        } break;

        case PokedexPageEntryType::DETAILS_1:
        {            
            pokedexStateComponent.mPokedexViewTimer->Update(dt);
            if (pokedexStateComponent.mPokedexViewTimer->HasTicked())
            {
                pokedexStateComponent.mPokedexViewTimer->Reset();
                pokedexStateComponent.mPokedexPokemonDescriptionCursorShowing = !pokedexStateComponent.mPokedexPokemonDescriptionCursorShowing;
                
                if (pokedexStateComponent.mPokedexPokemonDescriptionCursorShowing)
                {
                    WriteCharAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, '|', 18, 16, mWorld);
                }
                else
                {
                    DeleteCharAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, 18, 16, mWorld);
                }
            }

            if
            (
                IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||
                IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent)
            )
            {
                DisplayPokemonPokedexDescriptionForPage(1);
                pokedexStateComponent.mCurrentPageViewType = PokedexPageEntryType::DETAILS_2;
                pokedexStateComponent.mPokedexPokemonDescriptionCursorShowing = false;
                DeleteCharAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, 18, 16, mWorld);
            }
        } break;

        case PokedexPageEntryType::DETAILS_2:
        {
            if
            (
                IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||
                IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent)
            )
            {
                const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
                if (encounterStateComponent.mActiveEncounterType == EncounterType::NONE)
                {
                    //TODO: CompleteAndTransitionTo<PokedexViewFlowState>();
                }
                else
                {
                    DestroyPokedexPokemonEntryDisplay();
                    CreateChatbox(mWorld);
                    CompleteAndTransitionTo<PokemonNicknameQuestionTextEncounterFlowState>();
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PokedexPokemonEntryDisplayFlowState::DisplayLockedStatsText() const
{
    auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
    const auto& pokemonBaseStats = GetPokemonBaseStats(pokedexStateComponent.mSelectedPokemonName, mWorld);
    
    pokedexStateComponent.mPokedexInfoTextboxEntityId = CreatePokedexPokemonEntryDisplayTextbox(mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, pokemonBaseStats.mSpeciesName.GetString(), 9, 2, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, pokemonBaseStats.mPokedexPokemonType, 9, 4, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, "?", 13, 6, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, "?", 15, 6, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, "?", 16, 6, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, "?", 14, 8, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, "?", 15, 8, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, "?", 16, 8, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, GetFormattedPokemonIdString(pokemonBaseStats.mId), 4, 8, mWorld);
}

void PokedexPokemonEntryDisplayFlowState::DisplayPokemonBodyStatsText() const
{
    const auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
    const auto& pokemonBaseStats      = GetPokemonBaseStats(pokedexStateComponent.mSelectedPokemonName, mWorld);
    
    const auto pokemonBodyInfoTextSplitBySpace = StringSplit(pokemonBaseStats.mPokedexBodyStats, ' ');
    const auto heightFeet   = pokemonBodyInfoTextSplitBySpace[0];
    const auto heightInches = pokemonBodyInfoTextSplitBySpace[1];
    const auto weightLbs    = pokemonBodyInfoTextSplitBySpace[2] + ".0";

    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, heightFeet, 14 - heightFeet.size(), 6, mWorld);    
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, heightInches, 17 - heightInches.size(), 6, mWorld);    
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, weightLbs, 17 - weightLbs.size(), 8, mWorld);
}

void PokedexPokemonEntryDisplayFlowState::DisplayPokemonSprite() const
{
    auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
    
    const auto& pokemonBaseStats = GetPokemonBaseStats(pokedexStateComponent.mSelectedPokemonName, mWorld);

    pokedexStateComponent.mPokemonSpriteEntityId = LoadAndCreatePokemonSprite
    (
        pokemonBaseStats.mSpeciesName.GetString(),
        true,
        POKEMON_SPRITE_POSITION,
        POKEMON_SPRITE_SCALE,
        mWorld
    );

    SoundService::GetInstance().PlaySfx("cries/" + GetFormattedPokemonIdString(pokemonBaseStats.mId));
}

void PokedexPokemonEntryDisplayFlowState::DisplayPokemonPokedexDescriptionForPage(const int requestedPageNumber) const
{
    // This method forms the 2 pages of the pokedex description of the pokemon
    // It continually adds each word of the text (split by space), until the 
    // first page is full, and then moves on to the second page
    const auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
    const auto& pokemonBaseStats      = GetPokemonBaseStats(pokedexStateComponent.mSelectedPokemonName, mWorld);
        
    const auto descriptionTextSplitBySpace = StringSplit(pokemonBaseStats.mPokedexText, ' ');
    
    const auto LINE_CHARACTERS_CAPACITY = 18;
    
    std::vector<std::string> page{ "","","" };
    auto lineCharacterCounter = 0;
    auto lineCounter          = 0;
    auto fillingFirstPage     = true;

    for (const auto& word: descriptionTextSplitBySpace)
    {
        lineCharacterCounter += word.size() + 1; // + 1 for the trailing space character
        if (lineCharacterCounter > LINE_CHARACTERS_CAPACITY)
        {
            lineCharacterCounter = word.size() + 1;
            if (++lineCounter >= 3)
            {
                lineCounter = 0;
                fillingFirstPage = false;
            }
        }

        if 
        (
            (requestedPageNumber == 0 && fillingFirstPage) ||
            (requestedPageNumber == 1 && fillingFirstPage == false)
        )
        {
            page[lineCounter] += word + " ";
        }
    }

    DeleteTextAtTextboxRow(pokedexStateComponent.mPokedexInfoTextboxEntityId, 11, mWorld);
    DeleteTextAtTextboxRow(pokedexStateComponent.mPokedexInfoTextboxEntityId, 13, mWorld);
    DeleteTextAtTextboxRow(pokedexStateComponent.mPokedexInfoTextboxEntityId, 15, mWorld);
    
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, page[0], 1, 11, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, page[1], 1, 13, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexInfoTextboxEntityId, page[2], 1, 15, mWorld);
}

void PokedexPokemonEntryDisplayFlowState::DestroyPokedexPokemonEntryDisplay() const
{
    auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
    
    mWorld.DestroyEntity(pokedexStateComponent.mPokemonSpriteEntityId);
    mWorld.DestroyEntity(pokedexStateComponent.mPokedexBackgroundSpriteEntityId);
    DestroyGenericOrBareTextbox(pokedexStateComponent.mPokedexInfoTextboxEntityId, mWorld);
    
    pokedexStateComponent.mPokemonSpriteEntityId           = ecs::NULL_ENTITY_ID;
    pokedexStateComponent.mPokedexBackgroundSpriteEntityId = ecs::NULL_ENTITY_ID;
    pokedexStateComponent.mPokedexInfoTextboxEntityId      = ecs::NULL_ENTITY_ID;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
