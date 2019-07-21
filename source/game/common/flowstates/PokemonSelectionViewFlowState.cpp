//
//  PokemonSelectionViewFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 17/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokemonSelectionViewFlowState.h"
#include "MainMenuEncounterFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../resources/MeshUtils.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/components/WindowSingletonComponent.h"

#include <unordered_map>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PokemonSelectionViewFlowState::POKEMON_SELECTION_VIEW_BACKGROUND_MODEL_FILE_NAME   = "pokemon_selection_view_sprite.obj";
const std::string PokemonSelectionViewFlowState::POKEMON_SELECTION_VIEW_BACKGROUND_TEXTURE_FILE_NAME = "pokemon_selection_view_background.png";
const std::string PokemonSelectionViewFlowState::POKEMON_SPRITE_MODEL_NAME                           = "camera_facing_quad";
const std::string PokemonSelectionViewFlowState::POKEMON_SPRITE_ATLAS_TEXTURE_FILE_NAME              = "characters.png";

const glm::vec3 PokemonSelectionViewFlowState::POKEMON_SELECTION_VIEW_BACKGROUND_POSITION         = glm::vec3(0.0f, 0.0f, 0.01f);
const glm::vec3 PokemonSelectionViewFlowState::POKEMON_SELECTION_VIEW_BACKGROUND_SCALE            = glm::vec3(2.0f, 2.0f, 2.0f);
const glm::vec3 PokemonSelectionViewFlowState::POKEMON_SELECTION_VIEW_STATS_TEXTBOX_BASE_POSITION = glm::vec3(0.0f, 0.89f, -0.3f);
const glm::vec3 PokemonSelectionViewFlowState::POKEMON_SELECTION_OVERWORLD_SPRITE_BASE_POSITION   = glm::vec3(-0.55f, 0.9f,-0.4f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokemonSelectionViewFlowState::PokemonSelectionViewFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    CreatePokemonSelectionBackground();    

    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    for (auto i = 0U; i < playerStateComponent.mPlayerPokemonRoster.size(); ++i)
    {
        const auto& pokemon = *playerStateComponent.mPlayerPokemonRoster[i];
        
        CreatePokemonOverworldSprite
        (
            pokemon.mBaseStats.mOverworldSpriteType,
            i
        );
        
        LoadAndCreatePokemonHealthBar
        (
            pokemon.mHp/static_cast<float>(pokemon.mMaxHp),
            false,
            mWorld,
            true,
            i
        );
        
        LoadAndCreatePokemonSelectionViewBareHealthbarContainer
        (
            i,
            mWorld
        );
    }
    
    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    WriteTextAtTextboxCoords(mainChatboxEntityId, "Choose a POK^MON.", 1, 2, mWorld);       

    CreatePokemonStatsInvisibleTextbox();
}

void PokemonSelectionViewFlowState::VUpdate(const float)
{    
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>(); 

    if (IsActionTypeKeyTapped(VirtualActionType::A, inputStateComponent))
    {
        
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B, inputStateComponent))
    {
        // Destroy fight menu textbox
        DestroyActiveTextbox(mWorld);

        CompleteAndTransitionTo<MainMenuEncounterFlowState>();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PokemonSelectionViewFlowState::CreatePokemonSelectionBackground() const
{
    auto& pokemonSelectionViewEntities = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    pokemonSelectionViewEntities.mBackgroundEntityId = mWorld.CreateEntity();

    auto renderableComponent = std::make_unique<RenderableComponent>();

    const auto texturePath                      = ResourceLoadingService::RES_TEXTURES_ROOT + POKEMON_SELECTION_VIEW_BACKGROUND_TEXTURE_FILE_NAME;
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;

    const auto modelPath         = ResourceLoadingService::RES_MODELS_ROOT + POKEMON_SELECTION_VIEW_BACKGROUND_MODEL_FILE_NAME;
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = POKEMON_SELECTION_VIEW_BACKGROUND_POSITION;
    transformComponent->mScale    = POKEMON_SELECTION_VIEW_BACKGROUND_SCALE;

    mWorld.AddComponent<RenderableComponent>(pokemonSelectionViewEntities.mBackgroundEntityId, std::move(renderableComponent));
    mWorld.AddComponent<TransformComponent>(pokemonSelectionViewEntities.mBackgroundEntityId, std::move(transformComponent));
}

void PokemonSelectionViewFlowState::CreatePokemonStatsInvisibleTextbox() const
{
    const auto& playerStateComponent               = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& pokemonSelectionViewStateComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    const auto& guiStateSingletonComponent         = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& windowSingletonComponent           = mWorld.GetSingletonComponent<WindowSingletonComponent>();

    const auto guiTileHeight = guiStateSingletonComponent.mGlobalGuiTileHeight;
    const auto guiTileHeightAccountingForAspect = guiTileHeight * windowSingletonComponent.mAspectRatio;

    const auto pokemonSelectionViewTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_BARE_TEXTBOX,
        20,
        2 * playerStateComponent.mPlayerPokemonRoster.size(),
        POKEMON_SELECTION_VIEW_STATS_TEXTBOX_BASE_POSITION.x,
        POKEMON_SELECTION_VIEW_STATS_TEXTBOX_BASE_POSITION.y - (2.0f * guiTileHeightAccountingForAspect * (playerStateComponent.mPlayerPokemonRoster.size() - 1))/2.0f,
        POKEMON_SELECTION_VIEW_STATS_TEXTBOX_BASE_POSITION.z,
        mWorld
    );

    for (auto i = 0U; i < playerStateComponent.mPlayerPokemonRoster.size(); ++i)
    {
        const auto& pokemon = *playerStateComponent.mPlayerPokemonRoster[i];

        // Write pokemon's name
        WriteTextAtTextboxCoords
        (
            pokemonSelectionViewTextboxEntityId, 
            pokemon.mName.GetString(),
            3,
            i * 2,
            mWorld
        );

        // Write pokemon's level
        WriteTextAtTextboxCoords
        (
            pokemonSelectionViewTextboxEntityId,
            "=" + std::to_string(pokemon.mLevel),
            13,
            i * 2,
            mWorld
        );
        
        // Write pokemon's dead status or not
        if (pokemon.mHp <= 0)
        {
            WriteTextAtTextboxCoords
            (
                pokemonSelectionViewTextboxEntityId,
                "FNT",
                17,
                i * 2,
                mWorld
            );
        }
        
        // Write pokemon's current hp
        WriteTextAtTextboxCoords
        (
            pokemonSelectionViewTextboxEntityId,
            std::to_string(pokemon.mHp) + "/",
            16 - static_cast<int>(std::to_string(pokemon.mHp).size()),
            i * 2 + 1,
            mWorld
        );
        
        // Write pokemon's max hp
        WriteTextAtTextboxCoords
        (
            pokemonSelectionViewTextboxEntityId,
            std::to_string(pokemon.mMaxHp),
            20 - static_cast<int>(std::to_string(pokemon.mMaxHp).size()),
            i * 2 + 1,
            mWorld
        );
    }
    
    auto cursorComponent = std::make_unique<CursorComponent>();
    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = playerStateComponent.mPlayerPokemonRoster.size();

    cursorComponent->mCursorDisplayHorizontalTileOffset = 0;
    cursorComponent->mCursorDisplayVerticalTileOffset   = 1;
    
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;

    WriteCharAtTextboxCoords
    (
        pokemonSelectionViewTextboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        mWorld
    );

    cursorComponent->mWarp = true;

    mWorld.AddComponent<CursorComponent>(pokemonSelectionViewTextboxEntityId, std::move(cursorComponent));    
}


ecs::EntityId PokemonSelectionViewFlowState::CreatePokemonOverworldSprite(const OverworldPokemonSpriteType overworldSpriteType, const int row) const
{
    static const std::unordered_map<OverworldPokemonSpriteType, std::pair<int, int>> overworldSpriteTypeToAtlasCoords = 
    {
        { OverworldPokemonSpriteType::BALL,    {3, 44} },
        { OverworldPokemonSpriteType::BEAST,   {5, 44} },
        { OverworldPokemonSpriteType::BUG,     {7, 44} },
        { OverworldPokemonSpriteType::DRAGON,  {1, 45} },
        { OverworldPokemonSpriteType::FLYING,  {3, 45} },
        { OverworldPokemonSpriteType::FOSSIL,  {5, 45} },
        { OverworldPokemonSpriteType::GRASS,   {7, 45} },
        { OverworldPokemonSpriteType::NORMAL,  {1, 46} },
        { OverworldPokemonSpriteType::PIKACHU, {3, 46} },
        { OverworldPokemonSpriteType::WATER,   {5, 46} }
    };
    
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& windowComponent   = mWorld.GetSingletonComponent<WindowSingletonComponent>();


    auto spriteEntityId = mWorld.CreateEntity();

    const auto& overworldSpriteAtlasCoords = overworldSpriteTypeToAtlasCoords.at(overworldSpriteType);

    auto renderableComponent                    = std::make_unique<RenderableComponent>();
    renderableComponent->mTextureResourceId     = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + POKEMON_SPRITE_ATLAS_TEXTURE_FILE_NAME);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId          = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;
    renderableComponent->mAnimationsToMeshes[renderableComponent->mActiveAnimationNameId].push_back
    (
        LoadMeshFromAtlasTexCoords
        (
            overworldSpriteAtlasCoords.first,
            overworldSpriteAtlasCoords.second,
            8,
            64,
            false,
            POKEMON_SPRITE_MODEL_NAME
        )
    );

    auto secondFrameCoords = overworldSpriteAtlasCoords;
    if (++secondFrameCoords.first >= 8)
    {
        secondFrameCoords.first = 0;
        secondFrameCoords.second++;
    }

    renderableComponent->mAnimationsToMeshes[renderableComponent->mActiveAnimationNameId].push_back
    (
        LoadMeshFromAtlasTexCoords
        (
            secondFrameCoords.first,
            secondFrameCoords.second,
            8,
            64,
            false,
            POKEMON_SPRITE_MODEL_NAME
        )
    );

    const auto tileHeight = guiStateComponent.mGlobalGuiTileHeight;    
    const auto tileHeightAccountingForAspect = tileHeight * windowComponent.mAspectRatio;

    auto transformComponent    = std::make_unique<TransformComponent>();
    transformComponent->mScale = glm::vec3(guiStateComponent.mGlobalGuiTileWidth * 2, guiStateComponent.mGlobalGuiTileHeight * 2, -0.4f);   
    transformComponent->mPosition.x = POKEMON_SELECTION_OVERWORLD_SPRITE_BASE_POSITION.x;
    transformComponent->mPosition.y = POKEMON_SELECTION_OVERWORLD_SPRITE_BASE_POSITION.y - row * (tileHeightAccountingForAspect * 2);
    transformComponent->mPosition.z = POKEMON_SELECTION_OVERWORLD_SPRITE_BASE_POSITION.z;
    
    auto animationComponent = std::make_unique<AnimationTimerComponent>();
    animationComponent->mAnimationTimer = std::make_unique<Timer>(0.1f);    
    animationComponent->mAnimationTimer->Pause();

    mWorld.AddComponent<AnimationTimerComponent>(spriteEntityId, std::move(animationComponent));
    mWorld.AddComponent<RenderableComponent>(spriteEntityId, std::move(renderableComponent));
    mWorld.AddComponent<TransformComponent>(spriteEntityId, std::move(transformComponent));

    return spriteEntityId;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
