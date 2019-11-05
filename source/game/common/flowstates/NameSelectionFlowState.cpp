//
//  NameSelectionFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "GameIntroFlowState.h"
#include "NameSelectionFlowState.h"
#include "DeterminePokemonPlacementFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/NameSelectionStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TextboxComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/PokemonSelectionViewSpriteUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../resources/MeshUtils.h"

#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 NameSelectionFlowState::BACKGROUND_POSITION     = glm::vec3(0.0f, 0.0f, 0.01f);
const glm::vec3 NameSelectionFlowState::BACKGROUND_SCALE        = glm::vec3(2.0f, 2.0f, 2.0f);
const glm::vec3 NameSelectionFlowState::POKEMON_SPRITE_POSITION = glm::vec3(-0.5652f, 0.8925f, -0.1f);

const TileCoords NameSelectionFlowState::SELECTED_NAME_START_COORDS       = TileCoords(10, 2);
const TileCoords NameSelectionFlowState::SELECTED_POKEMON_NAME_END_COORDS = TileCoords(19, 2);
const TileCoords NameSelectionFlowState::SELECTED_TRAINER_NAME_END_COORDS = TileCoords(16, 2);
const TileCoords NameSelectionFlowState::END_BUTTON_COORDS                = TileCoords(8, 4);

const std::string NameSelectionFlowState::POKEMON_SPRITE_ATLAS_TEXTURE_FILE_NAME = "characters.png";
const std::string NameSelectionFlowState::POKEMON_SPRITE_MODEL_NAME              = "camera_facing_quad_hud_sub_atlas";

const int NameSelectionFlowState::CHARACTER_ATLAS_COLS = 8;
const int NameSelectionFlowState::CHARACTER_ATLAS_ROWS = 64;

const float NameSelectionFlowState::SPRITE_ANIMATION_FRAME_DURATION_MEDIUM = 0.16f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

NameSelectionFlowState::NameSelectionFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
    , mBackgroundCoverEntityId(ecs::NULL_ENTITY_ID)
    , mCharactersEnclosingTextboxEntityId(ecs::NULL_ENTITY_ID)
    , mTitleTextboxEntityId(ecs::NULL_ENTITY_ID)
    , mPokemonSpriteEntityId(ecs::NULL_ENTITY_ID)
    , mUppercaseMode(true)
{
    auto& nameSelectionStateComponent = mWorld.GetSingletonComponent<NameSelectionStateSingletonComponent>();
    nameSelectionStateComponent.mSelectedName = "";

    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    // Destroy Last Frame of Pokemon Caught animation
    if (encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId != ecs::NULL_ENTITY_ID)
    {
        mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId);
        encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = ecs::NULL_ENTITY_ID;
    }

    // Destroy Opponent pokemon sprite
    if (encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId != ecs::NULL_ENTITY_ID)
    {
        mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
        encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId = ecs::NULL_ENTITY_ID;
    }

    CreateBackground();
    CreateTextboxes();
    RedrawSelectedText();
    
    if (nameSelectionStateComponent.mNameSelectionMode == NameSelectionMode::POKEMON_NICKNAME)
    {
        CreatePokemonSprite(nameSelectionStateComponent.mPokemonToSelectNameFor->mName);
    }
}

void NameSelectionFlowState::VUpdate(const float)
{
    const auto characterSelectionTextboxEntityId = GetActiveTextboxEntityId(mWorld);
    
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    
    auto& cursorComponent = mWorld.GetComponent<CursorComponent>(characterSelectionTextboxEntityId);
    
    const auto cursorCol = cursorComponent.mCursorCol;
    const auto cursorRow = cursorComponent.mCursorRow;
    
    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        auto& nameSelectionStateComponent = mWorld.GetSingletonComponent<NameSelectionStateSingletonComponent>();
        
        // Case button pressed
        if (cursorRow == cursorComponent.mCursorRowCount - 1)
        {
            DestroyActiveTextbox(mWorld);
            CreateNameSelectionCharactersInvisibleTextbox(!mUppercaseMode, mWorld, cursorCol, cursorRow);
            mUppercaseMode = !mUppercaseMode;
        }
        // End button pressed
        else if (cursorCol == END_BUTTON_COORDS.mCol && cursorRow == END_BUTTON_COORDS.mRow)
        {
            FinishNamingFlow();
        }
        else
        {
            const auto maxAllowedSize = 1U + (nameSelectionStateComponent.mNameSelectionMode == NameSelectionMode::POKEMON_NICKNAME ?
                SELECTED_POKEMON_NAME_END_COORDS.mCol - SELECTED_NAME_START_COORDS.mCol :
                SELECTED_TRAINER_NAME_END_COORDS.mCol - SELECTED_NAME_START_COORDS.mCol);
            
            // Max name capacity reached
            if (nameSelectionStateComponent.mSelectedName.size() == maxAllowedSize)
            {
                RepositionCursorToEnd();
            }
            else
            {
                nameSelectionStateComponent.mSelectedName += GetCharAtTextboxCoords
                (
                    characterSelectionTextboxEntityId,
                    cursorComponent.mCursorDisplayHorizontalTileOffset + cursorComponent.mCursorDisplayHorizontalTileIncrements * cursorComponent.mCursorCol + 1,
                    cursorComponent.mCursorDisplayVerticalTileOffset + cursorComponent.mCursorDisplayVerticalTileIncrements * cursorComponent.mCursorRow,
                    mWorld
                );
                
                if (nameSelectionStateComponent.mSelectedName.size() == maxAllowedSize)
                {
                    RepositionCursorToEnd();
                }
            }
            
            RedrawSelectedText();
        }
            
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        auto& nameSelectionStateComponent = mWorld.GetSingletonComponent<NameSelectionStateSingletonComponent>();
        
        if (nameSelectionStateComponent.mSelectedName.size() > 0)
        {
            nameSelectionStateComponent.mSelectedName = nameSelectionStateComponent.mSelectedName.substr(0, nameSelectionStateComponent.mSelectedName.size() - 1);
            RedrawSelectedText();
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::START_BUTTON, inputStateComponent))
    {
        FinishNamingFlow();
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::SELECT_BUTTON, inputStateComponent))
    {
        DestroyActiveTextbox(mWorld);
        CreateNameSelectionCharactersInvisibleTextbox(!mUppercaseMode, mWorld, cursorCol, cursorRow);
        mUppercaseMode = !mUppercaseMode;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void NameSelectionFlowState::CreateBackground()
{
    mBackgroundCoverEntityId = LoadAndCreatePokemonSelectionViewBackgroundCover
    (
        BACKGROUND_POSITION,
        BACKGROUND_SCALE,
        mWorld
    );
}

void NameSelectionFlowState::CreatePokemonSprite(const StringId pokemonName)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
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
    
    mPokemonSpriteEntityId = mWorld.CreateEntity();
    
    const auto& pokemonBaseStats = GetPokemonBaseStats(pokemonName, mWorld);
    const auto& overworldSpriteAtlasCoords = overworldSpriteTypeToAtlasCoords.at(pokemonBaseStats.mOverworldSpriteType);
    
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
            CHARACTER_ATLAS_COLS,
            CHARACTER_ATLAS_ROWS,
            false,
            POKEMON_SPRITE_MODEL_NAME
         )
    );
    
    auto secondFrameCoords = overworldSpriteAtlasCoords;
    if (++secondFrameCoords.first >= CHARACTER_ATLAS_COLS)
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
            CHARACTER_ATLAS_COLS,
            CHARACTER_ATLAS_ROWS,
            false,
            POKEMON_SPRITE_MODEL_NAME
         )
    );
    
    auto transformComponent       = std::make_unique<TransformComponent>();
    transformComponent->mScale.x  = guiStateComponent.mGlobalGuiTileWidth * 2;
    transformComponent->mScale.y  = guiStateComponent.mGlobalGuiTileHeight * 2;
    transformComponent->mPosition = POKEMON_SPRITE_POSITION;
    
    auto animationComponent = std::make_unique<AnimationTimerComponent>();
    animationComponent->mAnimationTimer = std::make_unique<Timer>(SPRITE_ANIMATION_FRAME_DURATION_MEDIUM);
    animationComponent->mAnimationTimer->Resume();
    
    mWorld.AddComponent<AnimationTimerComponent>(mPokemonSpriteEntityId, std::move(animationComponent));
    mWorld.AddComponent<RenderableComponent>(mPokemonSpriteEntityId, std::move(renderableComponent));
    mWorld.AddComponent<TransformComponent>(mPokemonSpriteEntityId, std::move(transformComponent));
}

void NameSelectionFlowState::CreateTextboxes()
{
    const auto& nameSelectionStateComponent = mWorld.GetSingletonComponent<NameSelectionStateSingletonComponent>();
    
    mTitleTextboxEntityId = CreateNameSelectionTitleInvisibleTextbox(mWorld);
    
    if (nameSelectionStateComponent.mNameSelectionMode == NameSelectionMode::POKEMON_NICKNAME)
    {
        WriteTextAtTextboxCoords(mTitleTextboxEntityId, nameSelectionStateComponent.mPokemonToSelectNameFor->mName.GetString(), 4, 0, mWorld);
        WriteTextAtTextboxCoords(mTitleTextboxEntityId, "NICKNAME?", 1, 2, mWorld);
    }
    else if (nameSelectionStateComponent.mNameSelectionMode == NameSelectionMode::PLAYER_NAME)
    {
        WriteTextAtTextboxCoords(mTitleTextboxEntityId, "YOUR NAME?", 0, 0, mWorld);
    }
    else if (nameSelectionStateComponent.mNameSelectionMode == NameSelectionMode::RIVAL_NAME)
    {
        WriteTextAtTextboxCoords(mTitleTextboxEntityId, "RIVAL's NAME?", 0, 0, mWorld);
    }
    
    mCharactersEnclosingTextboxEntityId = CreateNameSelectionCharactersEnclosingTextbox(mWorld);
    CreateNameSelectionCharactersInvisibleTextbox(mUppercaseMode, mWorld);
}

void NameSelectionFlowState::RedrawSelectedText()
{
    const auto& nameSelectionStateComponent = mWorld.GetSingletonComponent<NameSelectionStateSingletonComponent>();
    
    const auto maxAllowedSize = 1 + (nameSelectionStateComponent.mNameSelectionMode == NameSelectionMode::POKEMON_NICKNAME ?
        SELECTED_POKEMON_NAME_END_COORDS.mCol - SELECTED_NAME_START_COORDS.mCol :
        SELECTED_TRAINER_NAME_END_COORDS.mCol - SELECTED_NAME_START_COORDS.mCol);
    
    for (int i = 0; i < maxAllowedSize; ++i)
    {        
        WriteSpecialCharacterAtTextboxCoords(mTitleTextboxEntityId, SpecialCharacter::CHARACTER_STAND_NORMAL, SELECTED_NAME_START_COORDS.mCol + i, SELECTED_NAME_START_COORDS.mRow, mWorld);
        DeleteCharAtTextboxCoords(mTitleTextboxEntityId, SELECTED_NAME_START_COORDS.mCol + i, SELECTED_NAME_START_COORDS.mRow - 1, mWorld);
    }
    
    auto characterCounter = 0;
    for (const auto character: nameSelectionStateComponent.mSelectedName)
    {
        WriteCharAtTextboxCoords(mTitleTextboxEntityId, character, SELECTED_NAME_START_COORDS.mCol + characterCounter, SELECTED_NAME_START_COORDS.mRow - 1, mWorld);
        characterCounter++;
    }
    
    if (characterCounter == maxAllowedSize)
    {
        WriteSpecialCharacterAtTextboxCoords(mTitleTextboxEntityId, SpecialCharacter::CHARACTER_STAND_UP, SELECTED_NAME_START_COORDS.mCol + maxAllowedSize - 1, SELECTED_NAME_START_COORDS.mRow, mWorld);
    }
    else
    {
        WriteSpecialCharacterAtTextboxCoords(mTitleTextboxEntityId, SpecialCharacter::CHARACTER_STAND_UP, SELECTED_NAME_START_COORDS.mCol + characterCounter, SELECTED_NAME_START_COORDS.mRow, mWorld);
    }
}

void NameSelectionFlowState::RepositionCursorToEnd()
{
    const auto characterSelectionTextboxEntityId = GetActiveTextboxEntityId(mWorld);
    auto& cursorComponent = mWorld.GetComponent<CursorComponent>(characterSelectionTextboxEntityId);
    
    DeleteCharAtTextboxCoords
    (
        characterSelectionTextboxEntityId,
        cursorComponent.mCursorDisplayHorizontalTileOffset + cursorComponent.mCursorDisplayHorizontalTileIncrements * cursorComponent.mCursorCol,
        cursorComponent.mCursorDisplayVerticalTileOffset + cursorComponent.mCursorDisplayVerticalTileIncrements * cursorComponent.mCursorRow,
        mWorld
    );
    
    cursorComponent.mCursorCol = END_BUTTON_COORDS.mCol;
    cursorComponent.mCursorRow = END_BUTTON_COORDS.mRow;
    
    WriteCharAtTextboxCoords
    (
        characterSelectionTextboxEntityId,
        '}',
        cursorComponent.mCursorDisplayHorizontalTileOffset + cursorComponent.mCursorDisplayHorizontalTileIncrements * cursorComponent.mCursorCol,
        cursorComponent.mCursorDisplayVerticalTileOffset + cursorComponent.mCursorDisplayVerticalTileIncrements * cursorComponent.mCursorRow,
        mWorld
    );
}

void NameSelectionFlowState::FinishNamingFlow()
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& nameSelectionStateComponent = mWorld.GetSingletonComponent<NameSelectionStateSingletonComponent>();
    auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    DestroyActiveTextbox(mWorld);
    
    mWorld.DestroyEntity(mBackgroundCoverEntityId);
    DestroyGenericOrBareTextbox(mCharactersEnclosingTextboxEntityId, mWorld);
    DestroyGenericOrBareTextbox(mTitleTextboxEntityId, mWorld);

    if (mPokemonSpriteEntityId != ecs::NULL_ENTITY_ID)
    {
        mWorld.DestroyEntity(mPokemonSpriteEntityId);
    }
    
    if (nameSelectionStateComponent.mNameSelectionMode == NameSelectionMode::POKEMON_NICKNAME)
    {
        if (nameSelectionStateComponent.mSelectedName.size() > 0)
        {
            nameSelectionStateComponent.mPokemonToSelectNameFor->mName = StringId(nameSelectionStateComponent.mSelectedName);
        }
        
        if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
        {
            CompleteAndTransitionTo<DeterminePokemonPlacementFlowState>();
        }
        else
        {
            CompleteOverworldFlow();
        }
    }
    else if (nameSelectionStateComponent.mNameSelectionMode == NameSelectionMode::PLAYER_NAME)
    {
        playerStateComponent.mPlayerTrainerName = StringId(nameSelectionStateComponent.mSelectedName);
        CompleteAndTransitionTo<GameIntroFlowState>();
    }
    else if (nameSelectionStateComponent.mNameSelectionMode == NameSelectionMode::RIVAL_NAME)
    {
        playerStateComponent.mRivalName = StringId(nameSelectionStateComponent.mSelectedName);
        CompleteAndTransitionTo<GameIntroFlowState>();
    }
}

void NameSelectionFlowState::DestroyBackground()
{
    mWorld.DestroyEntity(mBackgroundCoverEntityId);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
