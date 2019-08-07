//
//  EvolutionAnimationFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 06/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "EvolutionAnimationFlowState.h"
#include "NewMovesCheckFlowState.h"
#include "../components/EvolutionAnimationStateSingletonComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../overworld/components/TransitionAnimationStateSingletonComponent.h"
#include "../../rendering/components/RenderableComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 EvolutionAnimationFlowState::POKEMON_SPRITE_POSITION = glm::vec3(0.0f, 0.2f, -0.5f);
const glm::vec3 EvolutionAnimationFlowState::POKEMON_SPRITE_SCALE    = glm::vec3(-0.49f, 0.49f, 1.0f);

const float EvolutionAnimationFlowState::POKEMON_CRY_DELAY             = 2.0f;
const float EvolutionAnimationFlowState::POKEMON_TRANSITION_STEP_DELAY = 0.05f;

const int EvolutionAnimationFlowState::EVOLUTION_ANIMATION_STEP_COUNT   = 19;
const int EvolutionAnimationFlowState::EVOLUTION_ANIMATION_REPEAT_COUNT = 12;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

EvolutionAnimationFlowState::EvolutionAnimationFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    DestroyEncounterSprites(mWorld);
    ConfigureEvolutionAnimationState();
}

void EvolutionAnimationFlowState::VUpdate(const float dt)
{
    auto& evolutionAnimationStateComponent = mWorld.GetSingletonComponent<EvolutionAnimationStateSingletonComponent>();
    switch (evolutionAnimationStateComponent.mAnimationState)
    {
        case EvolutionAnimationState::NOT_STARTED: 
        {
            auto& playerStateComponent           = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
            const auto pokemonReadyToEvolveIndex = GetReadyToEvolvePokemonRosterIndex(playerStateComponent.mPlayerPokemonRoster);
            auto& pokemonReadyToEvolve           = *playerStateComponent.mPlayerPokemonRoster[pokemonReadyToEvolveIndex];
            
            playerStateComponent.mPlayerPokemonRoster[pokemonReadyToEvolveIndex] = std::move(pokemonReadyToEvolve.mEvolution);            

            DestroyActiveTextbox(mWorld);
            mWorld.DestroyEntity(evolutionAnimationStateComponent.mOldPokemonSpriteEntityId);
            mWorld.DestroyEntity(evolutionAnimationStateComponent.mNewPokemonSpriteEntityId);

            evolutionAnimationStateComponent.mOldPokemonSpriteEntityId = ecs::NULL_ENTITY_ID;
            evolutionAnimationStateComponent.mNewPokemonSpriteEntityId = ecs::NULL_ENTITY_ID;
            evolutionAnimationStateComponent.mNeedToCheckEvolutionNewMoves = true;

            playerStateComponent.mLeveledUpPokemonRosterIndex = pokemonReadyToEvolveIndex;
            CompleteAndTransitionTo<NewMovesCheckFlowState>();

        } break;

        case EvolutionAnimationState::SHOW_OLD_POKEMON_COLORED:
        {
            evolutionAnimationStateComponent.mEvolutionAnimationTimer->Update(dt);
            if (evolutionAnimationStateComponent.mEvolutionAnimationTimer->HasTicked())
            {
                evolutionAnimationStateComponent.mAnimationState          = EvolutionAnimationState::POKEMON_TRANSITION;
                evolutionAnimationStateComponent.mEvolutionAnimationTimer = std::make_unique<Timer>(POKEMON_TRANSITION_STEP_DELAY);

                auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
                transitionAnimationStateComponent.mBlackAndWhiteModeEnabled = true;
            }

        } break;

        case EvolutionAnimationState::POKEMON_TRANSITION: 
        {
            evolutionAnimationStateComponent.mEvolutionAnimationTimer->Update(dt);
            if (evolutionAnimationStateComponent.mEvolutionAnimationTimer->HasTicked())
            {
                evolutionAnimationStateComponent.mEvolutionAnimationTimer->Reset();
                
                if (++evolutionAnimationStateComponent.mEvolutionAnimationStepCounter < EVOLUTION_ANIMATION_STEP_COUNT)
                {
                    if (evolutionAnimationStateComponent.mEvolutionAnimationStepCounter < evolutionAnimationStateComponent.mEvolutionAnimationRepeatCounter * 2 &&
                        evolutionAnimationStateComponent.mEvolutionAnimationStepCounter % 2 != 1)
                    {
                        ToggleVisibilityOfNewPokemonSprite(true);
                        ToggleVisibilityOfOldPokemonSprite(false);
                    }
                    else
                    {
                        ToggleVisibilityOfNewPokemonSprite(false);
                        ToggleVisibilityOfOldPokemonSprite(true);
                    }
                }
                else
                {
                    if (++evolutionAnimationStateComponent.mEvolutionAnimationRepeatCounter < EVOLUTION_ANIMATION_REPEAT_COUNT)
                    {
                        evolutionAnimationStateComponent.mEvolutionAnimationStepCounter = 0;
                    }
                    else
                    {                        
                        ToggleVisibilityOfOldPokemonSprite(false);
                        ToggleVisibilityOfNewPokemonSprite(true);

                        evolutionAnimationStateComponent.mEvolutionAnimationTimer = std::make_unique<Timer>(POKEMON_CRY_DELAY);
                        evolutionAnimationStateComponent.mAnimationState = EvolutionAnimationState::SHOW_NEW_POKEMON_COLORED;                        
                    }
                }
            }
        } break;

        case EvolutionAnimationState::SHOW_NEW_POKEMON_COLORED: 
        {
            evolutionAnimationStateComponent.mEvolutionAnimationTimer->Update(dt);
            if (evolutionAnimationStateComponent.mEvolutionAnimationTimer->HasTicked())
            {
                evolutionAnimationStateComponent.mEvolutionAnimationTimer->Reset();

                auto& transitionAnimationStateComponent = mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>();
                transitionAnimationStateComponent.mBlackAndWhiteModeEnabled = false;

                evolutionAnimationStateComponent.mAnimationState = EvolutionAnimationState::EVOLUTION_FINISHED_TEXT;

                DestroyActiveTextbox(mWorld);

                const auto& playerStateComponent     = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
                const auto pokemonReadyToEvolveIndex = GetReadyToEvolvePokemonRosterIndex(playerStateComponent.mPlayerPokemonRoster);
                const auto& pokemonReadyToEvolve     = *playerStateComponent.mPlayerPokemonRoster[pokemonReadyToEvolveIndex];

                const auto& mainChatboxEntityId = CreateChatbox(mWorld);
                QueueDialogForChatbox
                (
                    mainChatboxEntityId, 
                    pokemonReadyToEvolve.mName.GetString() + " evolved#into " +
                    pokemonReadyToEvolve.mEvolution->mBaseSpeciesStats.mSpeciesName.GetString() + "!#+FREEZE", 
                    mWorld
                );
            }
        } break;

        case EvolutionAnimationState::EVOLUTION_FINISHED_TEXT:
        {            
            evolutionAnimationStateComponent.mEvolutionAnimationTimer->Update(dt);
            if (evolutionAnimationStateComponent.mEvolutionAnimationTimer->HasTicked())
            {
                const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
                if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
                {
                    evolutionAnimationStateComponent.mAnimationState = EvolutionAnimationState::NOT_STARTED;
                }
            }
        } break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void EvolutionAnimationFlowState::ConfigureEvolutionAnimationState() const
{
    const auto& playerStateComponent     = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto pokemonReadyToEvolveIndex = GetReadyToEvolvePokemonRosterIndex(playerStateComponent.mPlayerPokemonRoster);
    const auto& pokemonReadyToEvolve     = *playerStateComponent.mPlayerPokemonRoster[pokemonReadyToEvolveIndex];

    auto& evolutionAnimationStateComponent = mWorld.GetSingletonComponent<EvolutionAnimationStateSingletonComponent>();    
    evolutionAnimationStateComponent.mAnimationState = EvolutionAnimationState::SHOW_OLD_POKEMON_COLORED;
    evolutionAnimationStateComponent.mEvolutionAnimationStepCounter = 0;
    evolutionAnimationStateComponent.mEvolutionAnimationRepeatCounter = 0;

    evolutionAnimationStateComponent.mOldPokemonSpriteEntityId = LoadAndCreatePokemonSprite
    (
        pokemonReadyToEvolve.mBaseSpeciesStats.mSpeciesName,
        true,
        POKEMON_SPRITE_POSITION,
        POKEMON_SPRITE_SCALE,
        mWorld
    );
    
    evolutionAnimationStateComponent.mNewPokemonSpriteEntityId = LoadAndCreatePokemonSprite
    (
        pokemonReadyToEvolve.mEvolution->mBaseSpeciesStats.mSpeciesName,
        true,
        POKEMON_SPRITE_POSITION,
        POKEMON_SPRITE_SCALE,
        mWorld
    );

    evolutionAnimationStateComponent.mEvolutionAnimationTimer = std::make_unique<Timer>(POKEMON_CRY_DELAY);

    ToggleVisibilityOfNewPokemonSprite(false);
}

void EvolutionAnimationFlowState::ToggleVisibilityOfOldPokemonSprite(const bool visibility) const
{
    auto& evolutionAnimationStateComponent = mWorld.GetSingletonComponent<EvolutionAnimationStateSingletonComponent>();
    auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(evolutionAnimationStateComponent.mOldPokemonSpriteEntityId);
    renderableComponent.mVisibility = visibility;
}

void EvolutionAnimationFlowState::ToggleVisibilityOfNewPokemonSprite(const bool visibility) const
{
    auto& evolutionAnimationStateComponent = mWorld.GetSingletonComponent<EvolutionAnimationStateSingletonComponent>();
    auto& renderableComponent = mWorld.GetComponent<RenderableComponent>(evolutionAnimationStateComponent.mNewPokemonSpriteEntityId);
    renderableComponent.mVisibility = visibility;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////