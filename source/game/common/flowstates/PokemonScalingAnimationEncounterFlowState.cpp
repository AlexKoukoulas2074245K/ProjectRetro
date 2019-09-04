//
//  PokemonScalingAnimationEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MainMenuEncounterFlowState.h"
#include "OpponentPokemonStatusDisplayEncounterFlowState.h"
#include "PlayerPokemonTextIntroEncounterFlowState.h"
#include "PlayerPokemonWithdrawTextEncounterFlowState.h"
#include "PokemonScalingAnimationEncounterFlowState.h"
#include "TurnOverEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/PokemonSpriteScalingAnimationStateSingletonComponent.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../rendering/components/WindowSingletonComponent.h"
#include "../../resources/MeshUtils.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../../sound/SoundService.h"
#include "../utils/PokemonUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PokemonScalingAnimationEncounterFlowState::PLAYERS_POKEMON_SCALE_ORIGIN_BLOCK_POSITION   = glm::vec3(-0.390500486f, -0.275199711f, 0.05f);
const glm::vec3 PokemonScalingAnimationEncounterFlowState::OPPONENTS_POKEMON_SCALE_ORIGIN_BLOCK_POSITION = glm::vec3(0.381699532f, 0.273100376f, 0.05f);

const int PokemonScalingAnimationEncounterFlowState::INDIVIDUAL_POKEMON_SPRITE_ATLAS_COLS = 7;
const int PokemonScalingAnimationEncounterFlowState::INDIVIDUAL_POKEMON_SPRITE_ATLAS_ROWS = 7;

const float PokemonScalingAnimationEncounterFlowState::SCALING_STEP_DURATION = 0.1f;
const float PokemonScalingAnimationEncounterFlowState::SPRITE_FINAL_SCALE    = 0.49f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokemonScalingAnimationEncounterFlowState::PokemonScalingAnimationEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    auto& scalingStateComponent = mWorld.GetSingletonComponent<PokemonSpriteScalingAnimationStateSingletonComponent>();    
    scalingStateComponent.mScalingStep = scalingStateComponent.mScalingAnimationType == ScalingAnimationType::SCALING_DOWN ? 2 : 0;
    scalingStateComponent.mScalingStepDurationTimer = std::make_unique<Timer>(SCALING_STEP_DURATION);
    RepopulateScalingBlockEntities();
}

void PokemonScalingAnimationEncounterFlowState::VUpdate(const float dt)
{
    auto& scalingStateComponent   = mWorld.GetSingletonComponent<PokemonSpriteScalingAnimationStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
   
    // Sfx currently playing
    if (SoundService::GetInstance().IsPlayingSfx())
    {
        return;
    }
    // Sfx just finished playing
    else if (WasSfxPlayingOnPreviousUpdate())
    {                
        if (scalingStateComponent.mScaleOpponentPokemon)
        {
            CompleteAndTransitionTo<OpponentPokemonStatusDisplayEncounterFlowState>();
        }
        else
        {
            DestroyActiveTextbox(mWorld);

            if (encounterStateComponent.mPlayerChangedPokemonFromMainMenu)
            {
                encounterStateComponent.mPlayerChangedPokemonFromMainMenu = false;
                CompleteAndTransitionTo<TurnOverEncounterFlowState>();
            }
            else
            {
                CompleteAndTransitionTo<MainMenuEncounterFlowState>();
            }
        }    

        return;
    }    

    scalingStateComponent.mScalingStepDurationTimer->Update(dt);
    if (scalingStateComponent.mScalingStepDurationTimer->HasTicked())
    {
        scalingStateComponent.mScalingStepDurationTimer->Reset();

        if (scalingStateComponent.mScalingAnimationType == ScalingAnimationType::SCALING_UP)
        {   
            ++scalingStateComponent.mScalingStep;
            RepopulateScalingBlockEntities();

            if (scalingStateComponent.mScalingStep == 2)
            {         
                OnScaleUpFinished();
            }            
        }
        else
        {
            --scalingStateComponent.mScalingStep;
            RepopulateScalingBlockEntities();
            
            if (scalingStateComponent.mScalingStep == 0)
            {
                OnScaleDownFinished();
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PokemonScalingAnimationEncounterFlowState::OnScaleUpFinished()
{
    const auto& scalingStateComponent = mWorld.GetSingletonComponent<PokemonSpriteScalingAnimationStateSingletonComponent>();
    auto& encounterStateComponent     = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (scalingStateComponent.mScaleOpponentPokemon)
    {
        mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId).mScale =
            glm::vec3(SPRITE_FINAL_SCALE, SPRITE_FINAL_SCALE, 1.0f);

        SoundService::GetInstance().PlaySfx
        (
            "cries/" +
            GetFormattedPokemonIdString(encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex]->mBaseSpeciesStats.mId)
        );       
    }
    else
    {
        const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

        mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId).mScale =
            glm::vec3(SPRITE_FINAL_SCALE, SPRITE_FINAL_SCALE, 1.0f);

        SoundService::GetInstance().PlaySfx
        (
            "cries/" +
            GetFormattedPokemonIdString(playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex]->mBaseSpeciesStats.mId)
        );                         
    }
}

void PokemonScalingAnimationEncounterFlowState::OnScaleDownFinished()
{
    const auto& pokemonSelectionViewStateComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    auto& scalingStateComponent   = mWorld.GetSingletonComponent<PokemonSpriteScalingAnimationStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    for (const auto entityId : scalingStateComponent.mScalingBlockEntities)
    {
        mWorld.DestroyEntity(entityId);
    }
    
    scalingStateComponent.mScalingBlockEntities.clear();    
    
    if (scalingStateComponent.mScaleOpponentPokemon == false)    
    {
        mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId).mScale =
            glm::vec3(0.0f, 0.0f, 1.0f);
        
        mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId);
        encounterStateComponent.mViewObjects.mPlayerActiveSpriteEntityId = ecs::NULL_ENTITY_ID;
        encounterStateComponent.mActivePlayerPokemonRosterIndex          = pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex;

        DestroyActiveTextbox(mWorld);
        CompleteAndTransitionTo<PlayerPokemonTextIntroEncounterFlowState>();
    }    
}

void PokemonScalingAnimationEncounterFlowState::RepopulateScalingBlockEntities()
{    
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& activeOpponentPokemon   = *encounterStateComponent.mOpponentPokemonRoster.at(encounterStateComponent.mActiveOpponentPokemonRosterIndex);
    const auto& activePlayerPokemon     = *playerStateComponent.mPlayerPokemonRoster.at(encounterStateComponent.mActivePlayerPokemonRosterIndex);
    auto& scalingStateComponent         = mWorld.GetSingletonComponent<PokemonSpriteScalingAnimationStateSingletonComponent>();

    const auto pokemonSpriteImage =
        ResourceLoadingService::RES_TEXTURES_ROOT +
        (scalingStateComponent.mScaleOpponentPokemon ? "pkmnfront/" : "pkmnback/") +
        (scalingStateComponent.mScaleOpponentPokemon ? activeOpponentPokemon.mBaseSpeciesStats.mSpeciesName.GetString() : activePlayerPokemon.mBaseSpeciesStats.mSpeciesName.GetString()) +
        ".png";

    for (const auto entityId : scalingStateComponent.mScalingBlockEntities)
    {
        mWorld.DestroyEntity(entityId);
    }

    scalingStateComponent.mScalingBlockEntities.clear();

    switch (scalingStateComponent.mScalingStep)
    {
        case 0:
        {
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (                
                pokemonSpriteImage, 
                scalingStateComponent.mScaleOpponentPokemon, 
                0,
                0,
                3,
                6
            ));
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                -1,
                0,
                0,
                6
            ));
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                1,
                0,
                6,
                6
            ));
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                0,
                1,
                3,
                3
            ));

        } break;
        case 1:
        {
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                0,
                0,
                3,
                6
            ));
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                -1,
                0,
                1,
                6
            ));
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                -1,
                1,
                1,
                5
            ));
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                -1,
                2,
                1,
                3
            ));
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                0,
                1,
                3,
                5
            ));
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                0,
                2,
                3,
                3
            ));
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                1,
                0,
                5,
                6
            ));
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                1,
                1,
                5,
                5
            ));
            scalingStateComponent.mScalingBlockEntities.push_back(CreateScalingBlockEntity
            (
                pokemonSpriteImage,
                scalingStateComponent.mScaleOpponentPokemon,
                1,
                2,
                5,
                3
            ));
        } break;
        case 2: break;
    }
}

ecs::EntityId PokemonScalingAnimationEncounterFlowState::CreateScalingBlockEntity
(
    const std::string atlasPath,
    const bool isOpponentsPokemonSprite,
    const int colWRTOriginBlock,
    const int rowWRTOriginBlock,
    const int atlasCol,
    const int atlasRow
)
{    
    const auto& guiStateSingletonComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& windowSingletonComponent = mWorld.GetSingletonComponent<WindowSingletonComponent>();

    const auto guiTileWidth = guiStateSingletonComponent.mGlobalGuiTileWidth;
    const auto guiTileHeight = guiStateSingletonComponent.mGlobalGuiTileHeight;
    const auto guiTileHeightAccountingForAspect = guiTileHeight * windowSingletonComponent.mAspectRatio;

    const auto scalingBlockEntity = mWorld.CreateEntity();

    auto renderableComponent                    = std::make_unique<RenderableComponent>();
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(atlasPath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;
    
    LoadMeshFromAtlasTexCoordsAndAddToRenderableAnimations
    (
        atlasCol,
        atlasRow,
        INDIVIDUAL_POKEMON_SPRITE_ATLAS_COLS,
        INDIVIDUAL_POKEMON_SPRITE_ATLAS_ROWS,
        false,
        GUI_COMPONENTS_MODEL_NAME,
        renderableComponent->mActiveAnimationNameId,
        *renderableComponent
    );

    auto transformComponent    = std::make_unique<TransformComponent>();

    if (isOpponentsPokemonSprite)
    {
        transformComponent->mPosition.x = OPPONENTS_POKEMON_SCALE_ORIGIN_BLOCK_POSITION.x + colWRTOriginBlock * guiTileWidth;
        transformComponent->mPosition.y = OPPONENTS_POKEMON_SCALE_ORIGIN_BLOCK_POSITION.y + rowWRTOriginBlock * guiTileHeightAccountingForAspect;
        transformComponent->mPosition.z = OPPONENTS_POKEMON_SCALE_ORIGIN_BLOCK_POSITION.z;
    }
    else
    {
        transformComponent->mPosition.x = PLAYERS_POKEMON_SCALE_ORIGIN_BLOCK_POSITION.x + colWRTOriginBlock * guiTileWidth;
        transformComponent->mPosition.y = PLAYERS_POKEMON_SCALE_ORIGIN_BLOCK_POSITION.y + rowWRTOriginBlock * guiTileHeightAccountingForAspect;
        transformComponent->mPosition.z = PLAYERS_POKEMON_SCALE_ORIGIN_BLOCK_POSITION.z;
    }
    
    transformComponent->mScale = glm::vec3(guiStateSingletonComponent.mGlobalGuiTileWidth, guiStateSingletonComponent.mGlobalGuiTileHeight, 1.0f);

    mWorld.AddComponent<RenderableComponent>(scalingBlockEntity, std::move(renderableComponent));
    mWorld.AddComponent<TransformComponent>(scalingBlockEntity, std::move(transformComponent));

    return scalingBlockEntity;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
