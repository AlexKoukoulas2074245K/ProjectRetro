//
//  PokemonSelectionViewFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 19/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MainMenuEncounterFlowState.h"
#include "MainMenuOverworldFlowState.h"
#include "OpponentTrainerPokemonSummonTextEncounterFlowState.h"
#include "PlayerPokemonTextIntroEncounterFlowState.h"
#include "PlayerPokemonWithdrawTextEncounterFlowState.h"
#include "PokemonStatsDisplayViewFlowState.h"
#include "PokemonSelectionViewFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/MathUtils.h"
#include "../utils/PokemonSelectionViewSpriteUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
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

const std::string PokemonSelectionViewFlowState::POKEMON_SPRITE_MODEL_NAME                           = "camera_facing_quad";
const std::string PokemonSelectionViewFlowState::POKEMON_SPRITE_ATLAS_TEXTURE_FILE_NAME              = "characters.png";

const glm::vec3 PokemonSelectionViewFlowState::BACKGROUND_POSITION            = glm::vec3(0.0f, 0.0f, 0.01f);
const glm::vec3 PokemonSelectionViewFlowState::BACKGROUND_SCALE               = glm::vec3(2.0f, 2.0f, 2.0f);
const glm::vec3 PokemonSelectionViewFlowState::STATS_TEXTBOX_BASE_POSITION    = glm::vec3(0.0f, 0.885f, -0.3f);
const glm::vec3 PokemonSelectionViewFlowState::OVERWORLD_SPRITE_BASE_POSITION = glm::vec3(-0.55f, 0.9f,-0.4f);

const float PokemonSelectionViewFlowState::SPRITE_ANIMATION_FRAME_DURATION_SLOW   = 0.32f;
const float PokemonSelectionViewFlowState::SPRITE_ANIMATION_FRAME_DURATION_MEDIUM = 0.16f;
const float PokemonSelectionViewFlowState::SPRITE_ANIMATION_FRAME_DURATION_FAST   = 0.08f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokemonSelectionViewFlowState::PokemonSelectionViewFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    CreatePokemonSelectionBackground();    
    CreateIndividualPokemonSprites();
    CreatePokemonSelectionViewMainTextbox();
    CreatePokemonStatsInvisibleTextbox();
}

void PokemonSelectionViewFlowState::VUpdate(const float)
{        
    auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    if (pokemonSelectionViewComponent.mNoWillToFightTextFlowActive)
    {
        const auto& guiStateSingletonComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
        if (guiStateSingletonComponent.mActiveTextboxesStack.size() == 2)
        {
            pokemonSelectionViewComponent.mNoWillToFightTextFlowActive = false;

            // Destroy pokemon attributes textbox
            DestroyActiveTextbox(mWorld);

            CreatePokemonSelectionViewMainTextbox();

            CreatePokemonStatsInvisibleTextbox();

            pokemonSelectionViewComponent.mPokemonHasBeenSelected = false;
        }
    }
    else if (pokemonSelectionViewComponent.mIndexSwapFlowActive)
    {
        PokemonSelectionViewIndexSwapFlow();
    }
    else
    {
        if (pokemonSelectionViewComponent.mPokemonHasBeenSelected)
        {
            PokemonSelectedFlow();
        }
        else
        {
            PokemonNotSelectedFlow();
        }
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PokemonSelectionViewFlowState::PokemonSelectedFlow()
{
    const auto& inputStateComponent     = mWorld.GetSingletonComponent<InputStateSingletonComponent>();    
    auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {        
        const auto commandFirstFourLetters = GetCursorCommandTextFirstFourLetters();

        if (commandFirstFourLetters == "SWIT")
        {
            if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::OVERWORLD)
            {
                PokemonRosterIndexSwapFlow();
            }
            else
            {
                SwitchPokemonFlow();
            }            
        }
        else if (commandFirstFourLetters == "STAT")
        {
            DisplayPokemonDetailedStatsFlow();
        }
        else if (commandFirstFourLetters == "CANC")
        {
            CancelPokemonSelectionFlow();
        }        
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {                        
        // Destroy pokemon selected textbox
        DestroyActiveTextbox(mWorld);
        
        const auto& pokemonSelectionViewCursorComponent = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));

        const auto pokemonSpriteEntityId = pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewCursorComponent.mCursorRow][0];
        auto& animationTimerComponent    = mWorld.GetComponent<AnimationTimerComponent>(pokemonSpriteEntityId);

        animationTimerComponent.mAnimationTimer->Resume();

        pokemonSelectionViewComponent.mPokemonHasBeenSelected = false;
    }
}

void PokemonSelectionViewFlowState::PokemonNotSelectedFlow()
{
    const auto& inputStateComponent     = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));                
        const auto pokemonSpriteEntityId = pokemonSelectionViewComponent.mPokemonSpriteEntityIds[cursorComponent.mCursorRow][0];
        auto& animationTimerComponent    = mWorld.GetComponent<AnimationTimerComponent>(pokemonSpriteEntityId);

        animationTimerComponent.mAnimationTimer->Reset();
        animationTimerComponent.mAnimationTimer->Pause();
                
        if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::ENCOUNTER_AFTER_POKEMON_FAINTED)
        {
            SwitchPokemonFlow();
        }
        else
        {
            CreatePokemonSelectionViewSelectionTextbox
            (
                pokemonSelectionViewComponent.mCreationSourceType != PokemonSelectionViewCreationSourceType::OVERWORLD,
                mWorld
            );
        }
        
        pokemonSelectionViewComponent.mPokemonHasBeenSelected = true;
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
        const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
        const auto& activePlayerPokemon  = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
        
        if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::OVERWORLD)
        {
            DestroyPokemonSelectionView();
            CompleteAndTransitionTo<MainMenuOverworldFlowState>();
        }
        else if
        (
            pokemonSelectionViewComponent.mCreationSourceType != PokemonSelectionViewCreationSourceType::ENCOUNTER_AFTER_POKEMON_FAINTED ||
            activePlayerPokemon.mHp > 0
        )
        {
            DestroyPokemonSelectionView();

            if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::ENCOUNTER_FROM_MAIN_MENU)
            {
                CompleteAndTransitionTo<MainMenuEncounterFlowState>();
            }
            else if (activePlayerPokemon.mHp > 0)
            {
                encounterStateComponent.mPlayerDecidedToChangePokemonBeforeNewOpponentPokemonIsSummoned = false;
                CompleteAndTransitionTo<OpponentTrainerPokemonSummonTextEncounterFlowState>();
            }                       
        }        
    }
    else if 
    (
        IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent) ||
        IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent)
    )
    {
        const auto& cursorComponent           = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));        
        const auto& currentPokemonUnderCursor = *mWorld.GetSingletonComponent<PlayerStateSingletonComponent>().mPlayerPokemonRoster[cursorComponent.mCursorRow];

        const auto rosterSize = static_cast<int>(pokemonSelectionViewComponent.mPokemonSpriteEntityIds.size());
        auto previousPokemonUnderCursorRosterIndex = IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent) ? cursorComponent.mCursorRow + 1 : cursorComponent.mCursorRow - 1;

        if (previousPokemonUnderCursorRosterIndex < 0)
        {
            previousPokemonUnderCursorRosterIndex = rosterSize - 1;
        }
        else if (previousPokemonUnderCursorRosterIndex >= rosterSize)
        {
            previousPokemonUnderCursorRosterIndex = 0;
        }

        const auto previousPokemonUnderCursorSpriteEntityId     = pokemonSelectionViewComponent.mPokemonSpriteEntityIds[previousPokemonUnderCursorRosterIndex][0];
        auto& previousPokemonUnderCursorAnimationTimerComponent = mWorld.GetComponent<AnimationTimerComponent>(previousPokemonUnderCursorSpriteEntityId);
        previousPokemonUnderCursorAnimationTimerComponent.mAnimationTimer->Reset();
        previousPokemonUnderCursorAnimationTimerComponent.mAnimationTimer->Pause();

        if (currentPokemonUnderCursor.mHp > 0)
        {
            const auto pokemonSpriteEntityId = pokemonSelectionViewComponent.mPokemonSpriteEntityIds[cursorComponent.mCursorRow][0];
            auto& animationTimerComponent    = mWorld.GetComponent<AnimationTimerComponent>(pokemonSpriteEntityId);            
            animationTimerComponent.mAnimationTimer->Resume();
        }

        pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex = cursorComponent.mCursorRow;
    }    
}

void PokemonSelectionViewFlowState::PokemonSelectionViewIndexSwapFlow()
{
    const auto& inputStateComponent     = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        pokemonSelectionViewComponent.mIndexSwapFlowActive = false;
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        pokemonSelectionViewComponent.mIndexSwapFlowActive = false;
    }
    else if 
    (
        IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent) ||
        IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent)
    )
}

void PokemonSelectionViewFlowState::DisplayPokemonDetailedStatsFlow()
{
    // Destroy pokemon selected textbox
    DestroyActiveTextbox(mWorld);

    DestroyPokemonSelectionView();

    mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>().mPokemonHasBeenSelected = false;
    
    CompleteAndTransitionTo<PokemonStatsDisplayViewFlowState>();
}

void PokemonSelectionViewFlowState::SwitchPokemonFlow()
{    
    auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    auto& encounterStateComponent       = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& playerStateComponent          = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& selectedPokemon         = *playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex];

    if (selectedPokemon.mHp <= 0 || pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex == encounterStateComponent.mActivePlayerPokemonRosterIndex)
    {
        if (pokemonSelectionViewComponent.mCreationSourceType != PokemonSelectionViewCreationSourceType::ENCOUNTER_AFTER_POKEMON_FAINTED)
        {
            // Destroy pokemon selection textbox
            DestroyActiveTextbox(mWorld);
        }

        // Destroy pokemon stats textbox
        DestroyActiveTextbox(mWorld);

        // Destroy Choose a pokemon textbox
        DestroyActiveTextbox(mWorld);

        // Recreate pokemon stats textbox
        CreatePokemonStatsInvisibleTextbox();
        
        const auto mainChatboxEntityId = CreateChatbox(mWorld);

        if (pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex == encounterStateComponent.mActivePlayerPokemonRosterIndex)
        {
            if (selectedPokemon.mHp <= 0)
            {
                QueueDialogForChatbox(mainChatboxEntityId, "There's no will#to fight!#+END", mWorld);
            }
            else
            {
                QueueDialogForChatbox(mainChatboxEntityId, selectedPokemon.mName.GetString() + " is#already out!#+END", mWorld);
            }
        }
        else
        {
            QueueDialogForChatbox(mainChatboxEntityId, "There's no will#to fight!#+END", mWorld);
        }        

        pokemonSelectionViewComponent.mNoWillToFightTextFlowActive = true;
    }
    else
    {
        DestroyPokemonSelectionView();
        
        auto& currentlyActivePokemon = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];
        if (currentlyActivePokemon.mStatus == PokemonStatus::CONFUSED)
        {
            currentlyActivePokemon.mStatus                           = PokemonStatus::NORMAL;
            currentlyActivePokemon.mNumberOfRoundsUntilConfusionEnds = 0;
        }
        
        if (pokemonSelectionViewComponent.mCreationSourceType != PokemonSelectionViewCreationSourceType::ENCOUNTER_AFTER_POKEMON_FAINTED)
        {
            encounterStateComponent.mIsOpponentsTurn = false;
            encounterStateComponent.mPlayerChangedPokemonFromMainMenu = true;
            DestroyActiveTextbox(mWorld);
            CompleteAndTransitionTo<PlayerPokemonWithdrawTextEncounterFlowState>();
        }
        else
        {
            if (encounterStateComponent.mOpponentPendingSummoning)
            {
                CompleteAndTransitionTo<OpponentTrainerPokemonSummonTextEncounterFlowState>();
            }
            else
            {
                encounterStateComponent.mActivePlayerPokemonRosterIndex = pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex;
                CompleteAndTransitionTo<PlayerPokemonTextIntroEncounterFlowState>();
            }            
        }        
    }
}

void PokemonSelectionViewFlowState::PokemonRosterIndexSwapFlow()
{    
    // Destroy pokemon selection textbox
    DestroyActiveTextbox(mWorld);

    // Destroy pokemon stats textbox
    DestroyActiveTextbox(mWorld);

    // Destroy Choose a pokemon textbox
    DestroyActiveTextbox(mWorld);

    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    WriteTextAtTextboxCoords(mainChatboxEntityId, "Move POK^MON", 1, 2, mWorld);
    WriteTextAtTextboxCoords(mainChatboxEntityId, "where?", 1, 4, mWorld);

    // Recreate pokemon stats textbox
    CreatePokemonStatsInvisibleTextbox();    
}

void PokemonSelectionViewFlowState::CancelPokemonSelectionFlow()
{
    auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    // Destroy pokemon selected textbox
    DestroyActiveTextbox(mWorld);

    DestroyPokemonSelectionView();

    if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::OVERWORLD)
    {
        CompleteAndTransitionTo<MainMenuOverworldFlowState>();
    }
    else
    {
        CompleteAndTransitionTo<MainMenuEncounterFlowState>();
    }
    
    pokemonSelectionViewComponent.mPokemonHasBeenSelected = false;
}

void PokemonSelectionViewFlowState::CreatePokemonSelectionBackground() const
{
    auto& pokemonSelectionViewEntities = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    pokemonSelectionViewEntities.mBackgroundEntityId = LoadAndCreateBackgroundCover
    (
        BACKGROUND_POSITION, 
        BACKGROUND_SCALE, 
        mWorld
    );
}

void PokemonSelectionViewFlowState::CreateIndividualPokemonSprites() const
{
    const auto& playerStateComponent         = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& pokemonSelectionViewStateComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    auto& pokemonSpriteEntityIds             = pokemonSelectionViewStateComponent.mPokemonSpriteEntityIds;

    // Make sure that selection view cursor does not point to an index bigger than the party.
    // This could happen if during the last pokemon selection view the last pokemon was pointed to,
    // however before the view was opened again that last pokemon got released
    pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex = math::Min
    (
        static_cast<int>(playerStateComponent.mPlayerPokemonRoster.size()) - 1,
        pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex
    );

    pokemonSelectionViewStateComponent.mIndexSwapFlowActive         = false;
    pokemonSelectionViewStateComponent.mNoWillToFightTextFlowActive = false;

    pokemonSpriteEntityIds.clear();
    pokemonSpriteEntityIds.resize(playerStateComponent.mPlayerPokemonRoster.size());

    for (auto i = 0U; i < playerStateComponent.mPlayerPokemonRoster.size(); ++i)
    {        
        const auto& pokemon = *playerStateComponent.mPlayerPokemonRoster[i];
        
        pokemonSpriteEntityIds[i][0] = CreatePokemonOverworldSprite
        (
            pokemon.mBaseSpeciesStats.mOverworldSpriteType,
            static_cast<float>(pokemon.mHp)/pokemon.mMaxHp,
            i
        );
        
        pokemonSpriteEntityIds[i][1] = LoadAndCreatePokemonHealthBar
        (
            pokemon.mHp/static_cast<float>(pokemon.mMaxHp),
            false,
            mWorld,
            false,
            true,
            i
        );
        
        pokemonSpriteEntityIds[i][2] = LoadAndCreatePokemonSelectionViewBareHealthbarContainer
        (
            i,
            mWorld
        );
    }    
}

void PokemonSelectionViewFlowState::CreatePokemonSelectionViewMainTextbox() const
{
    const auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    const auto mainChatboxEntityId            = CreateChatbox(mWorld);

    if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::ENCOUNTER_AFTER_POKEMON_FAINTED)
    {
        WriteTextAtTextboxCoords(mainChatboxEntityId, "Bring out which", 1, 2, mWorld);
        WriteTextAtTextboxCoords(mainChatboxEntityId, "POK^MON?", 1, 4, mWorld);
    }
    else
    {
        WriteTextAtTextboxCoords(mainChatboxEntityId, "Choose a POK^MON.", 1, 2, mWorld);
    }
}

void PokemonSelectionViewFlowState::CreatePokemonStatsInvisibleTextbox() const
{
    const auto& playerStateComponent               = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& guiStateSingletonComponent         = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& windowSingletonComponent           = mWorld.GetSingletonComponent<WindowSingletonComponent>();
    auto& pokemonSelectionViewStateComponent       = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    const auto guiTileHeight = guiStateSingletonComponent.mGlobalGuiTileHeight;
    const auto guiTileHeightAccountingForAspect = guiTileHeight * windowSingletonComponent.mAspectRatio;

    pokemonSelectionViewStateComponent.mPokemonHasBeenSelected = false;

    const auto pokemonSelectionViewTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_BARE_TEXTBOX,
        20,
        2 * playerStateComponent.mPlayerPokemonRoster.size(),
        STATS_TEXTBOX_BASE_POSITION.x,
        STATS_TEXTBOX_BASE_POSITION.y - (2.0f * guiTileHeightAccountingForAspect * (playerStateComponent.mPlayerPokemonRoster.size() - 1))/2.0f,
        STATS_TEXTBOX_BASE_POSITION.z,
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
        
        if
        (
            (pokemon.mStatus != PokemonStatus::NORMAL &&
            pokemon.mStatus != PokemonStatus::CONFUSED) ||
            pokemon.mHp <= 0
        )
        {
            WriteTextAtTextboxCoords
            (
                pokemonSelectionViewTextboxEntityId,
                GetFormattedPokemonStatus(pokemon.mHp, pokemon.mStatus),
                17,
                i * 2,
                mWorld
            );
        }
        
        
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

    // Start animation on sprite under cursor
    const auto pokemonSpriteEntityId = pokemonSelectionViewStateComponent.mPokemonSpriteEntityIds[pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex][0];
    auto& animationTimerComponent    = mWorld.GetComponent<AnimationTimerComponent>(pokemonSpriteEntityId);

    if (playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex]->mHp > 0)
    {
        animationTimerComponent.mAnimationTimer->Resume();
    }    

    mWorld.AddComponent<CursorComponent>(pokemonSelectionViewTextboxEntityId, std::move(cursorComponent));        
}

void PokemonSelectionViewFlowState::DestroyPokemonSelectionView() const
{
    // Destroy Invisible Stats Textbox
    DestroyActiveTextbox(mWorld);

    // Destroy Choose Pokemon Textbox
    DestroyActiveTextbox(mWorld);

    // Destroy individual pokemon sprites
    const auto& playerStateComponent         = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& pokemonSelectionViewStateComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    auto& pokemonSpriteEntityIds             = pokemonSelectionViewStateComponent.mPokemonSpriteEntityIds;
        
    for (auto i = 0U; i < playerStateComponent.mPlayerPokemonRoster.size(); ++i)
    {
        mWorld.DestroyEntity(pokemonSpriteEntityIds[i][0]);
        mWorld.DestroyEntity(pokemonSpriteEntityIds[i][1]);
        mWorld.DestroyEntity(pokemonSpriteEntityIds[i][2]);

        pokemonSpriteEntityIds[i][0] = ecs::NULL_ENTITY_ID;
        pokemonSpriteEntityIds[i][1] = ecs::NULL_ENTITY_ID;
        pokemonSpriteEntityIds[i][2] = ecs::NULL_ENTITY_ID;
    }

    pokemonSpriteEntityIds.clear();

    // Destroy background
    mWorld.DestroyEntity(pokemonSelectionViewStateComponent.mBackgroundEntityId);
    pokemonSelectionViewStateComponent.mBackgroundEntityId = ecs::NULL_ENTITY_ID;
}

std::string PokemonSelectionViewFlowState::GetCursorCommandTextFirstFourLetters() const
{
    const auto activeTextboxEntityId   = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent        = mWorld.GetComponent<CursorComponent>(activeTextboxEntityId);
    const auto& activeTextboxComponent = mWorld.GetComponent<TextboxComponent>(activeTextboxEntityId);

    std::string commandFirstFourLetters;
    commandFirstFourLetters.push_back(activeTextboxComponent.mTextContent[cursorComponent.mCursorRow * 2 + 1][2].mCharacter);
    commandFirstFourLetters.push_back(activeTextboxComponent.mTextContent[cursorComponent.mCursorRow * 2 + 1][3].mCharacter);
    commandFirstFourLetters.push_back(activeTextboxComponent.mTextContent[cursorComponent.mCursorRow * 2 + 1][4].mCharacter);
    commandFirstFourLetters.push_back(activeTextboxComponent.mTextContent[cursorComponent.mCursorRow * 2 + 1][5].mCharacter);

    return commandFirstFourLetters;
}

ecs::EntityId PokemonSelectionViewFlowState::CreatePokemonOverworldSprite
(
    const OverworldPokemonSpriteType overworldSpriteType, 
    const float healthRemainingProportion, 
    const int row
) const
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
    transformComponent->mPosition.x = OVERWORLD_SPRITE_BASE_POSITION.x;
    transformComponent->mPosition.y = OVERWORLD_SPRITE_BASE_POSITION.y - row * (tileHeightAccountingForAspect * 2);
    transformComponent->mPosition.z = OVERWORLD_SPRITE_BASE_POSITION.z;
    
    auto animationComponent = std::make_unique<AnimationTimerComponent>();

    auto animationFrameDuration = SPRITE_ANIMATION_FRAME_DURATION_FAST;
    if (healthRemainingProportion <= 0.25f)
    {
        animationFrameDuration = SPRITE_ANIMATION_FRAME_DURATION_SLOW;
    }
    else if (healthRemainingProportion <= 0.5f)
    {
        animationFrameDuration = SPRITE_ANIMATION_FRAME_DURATION_MEDIUM;
    }

    animationComponent->mAnimationTimer = std::make_unique<Timer>(animationFrameDuration);     
    animationComponent->mAnimationTimer->Pause();        

    mWorld.AddComponent<AnimationTimerComponent>(spriteEntityId, std::move(animationComponent));
    mWorld.AddComponent<RenderableComponent>(spriteEntityId, std::move(renderableComponent));
    mWorld.AddComponent<TransformComponent>(spriteEntityId, std::move(transformComponent));

    return spriteEntityId;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
