//
//  PokemonSelectionViewFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 19/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ItemMenuFlowState.h"
#include "MainMenuEncounterFlowState.h"
#include "MainMenuOverworldFlowState.h"
#include "OpponentTrainerPokemonSummonTextEncounterFlowState.h"
#include "PlayerPokemonTextIntroEncounterFlowState.h"
#include "PlayerPokemonWithdrawTextEncounterFlowState.h"
#include "PokemonStatsDisplayViewFlowState.h"
#include "PokemonSelectionViewFlowState.h"
#include "TurnOverEncounterFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/PokemonSelectionViewStateSingletonComponent.h"
#include "../components/PokemonStatsDisplayViewStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/MathUtils.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/PokemonSelectionViewSpriteUtils.h"
#include "../utils/PokemonItemsUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../resources/MeshUtils.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/components/WindowSingletonComponent.h"
#include "../../sound/SoundService.h"

#include <unordered_map>
#include <utility>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string PokemonSelectionViewFlowState::TEXTBOX_CLICK_SFX_NAME                 = "general/textbox_click";
const std::string PokemonSelectionViewFlowState::POKEMON_SWAP_SFX_NAME                  = "general/pokemon_swap";
const std::string PokemonSelectionViewFlowState::ITEM_HEAL_UP_SFX_NAME                  = "general/item_heal_up";
const std::string PokemonSelectionViewFlowState::POKEMON_SPRITE_MODEL_NAME              = "camera_facing_quad_hud_sub_atlas";
const std::string PokemonSelectionViewFlowState::POKEMON_SPRITE_ATLAS_TEXTURE_FILE_NAME = "characters.png";

const glm::vec3 PokemonSelectionViewFlowState::BACKGROUND_POSITION                = glm::vec3(0.0f, 0.0f, 0.01f);
const glm::vec3 PokemonSelectionViewFlowState::BACKGROUND_SCALE                   = glm::vec3(2.0f, 2.0f, 2.0f);
const glm::vec3 PokemonSelectionViewFlowState::STATS_TEXTBOX_BASE_POSITION        = glm::vec3(0.0f, 0.885f, -0.3f);
const glm::vec3 PokemonSelectionViewFlowState::OVERWORLD_SPRITE_BASE_POSITION     = glm::vec3(-0.55f, 0.89f,-0.4f);
const glm::vec3 PokemonSelectionViewFlowState::ITEM_USAGE_RESULT_CHATBOX_POSITION = glm::vec3(0.0f, -0.6701f, -0.2f);

const float PokemonSelectionViewFlowState::SPRITE_ANIMATION_FRAME_DURATION_SLOW   = 0.32f;
const float PokemonSelectionViewFlowState::SPRITE_ANIMATION_FRAME_DURATION_MEDIUM = 0.16f;
const float PokemonSelectionViewFlowState::SPRITE_ANIMATION_FRAME_DURATION_FAST   = 0.08f;
const float PokemonSelectionViewFlowState::INDEX_SWAP_ENTRY_DELAY                 = 0.3f;

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

void PokemonSelectionViewFlowState::VUpdate(const float dt)
{       
    auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    if (pokemonSelectionViewComponent.mOperationState == PokemonSelectionViewOperationState::HEALING_UP)
    {
        HealingUpFlow(dt);
    }
    else if (pokemonSelectionViewComponent.mOperationState == PokemonSelectionViewOperationState::INVALID_OPERATION)
    {
        InvalidOperationFlow();
    }
    else if (pokemonSelectionViewComponent.mOperationState == PokemonSelectionViewOperationState::INDEX_SWAP_FLOW)
    {
        PokemonSelectionViewIndexSwapFlow(dt);
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

void PokemonSelectionViewFlowState::HealingUpFlow(const float dt)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    // Get selected item stats
    const auto& itemBagEntry = playerStateComponent.mPlayerBag.at(playerStateComponent.mPreviousItemMenuItemOffset + playerStateComponent.mPreviousItemMenuCursorRow);
    const auto itemName = itemBagEntry.mItemName;
    const auto& selectedItemStats = GetItemStats(itemBagEntry.mItemName, mWorld);

    // Get selected pokemons
    auto& selectedPokemon = *playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex];

    if
    (
        (encounterStateComponent.mActiveEncounterType == EncounterType::NONE && guiStateComponent.mActiveTextboxesStack.size() == 4) ||
        (encounterStateComponent.mActiveEncounterType != EncounterType::NONE && guiStateComponent.mActiveTextboxesStack.size() == 3)
    )
    {
        const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
        if
        (
            IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||
            IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent)
        )
        {
            RemoveItemFromBag(selectedItemStats.mName, mWorld);

            if (encounterStateComponent.mActiveEncounterType == EncounterType::NONE)
            {                
                DestroyActiveTextbox(mWorld);
            }
            
            DestroyActiveTextbox(mWorld);
            DestroyPokemonSelectionView();
        
            if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
            {
                CreateChatbox(mWorld);
                encounterStateComponent.mIsOpponentsTurn = false;
                RefreshPlayerPokemonDisplayStats();
                CompleteAndTransitionTo<TurnOverEncounterFlowState>();
            }
            else
            {
                CreateOverworldMainMenuTextbox(mWorld, HasMilestone(milestones::RECEIVED_POKEDEX, mWorld), playerStateComponent.mPreviousMainMenuCursorRow);
                CompleteAndTransitionTo<ItemMenuFlowState>();
            }            
        }
    }
    else
    {
        const auto healthChangeSpeed = CalculateHealthDepletionSpeed(selectedPokemon);
        
        pokemonSelectionViewComponent.mFloatPokemonHealth += healthChangeSpeed * dt;
        pokemonSelectionViewComponent.mHealthToRestoreCapacity -= healthChangeSpeed * dt;
        
        // Destroy pokemon stats textbox
        DestroyActiveTextbox(mWorld);
        
        // Destroy "Use item on which pokemon?" textbox
        DestroyActiveTextbox(mWorld);

        // Recreate pokemon stats textbox
        CreatePokemonStatsInvisibleTextbox();
        
        const auto mainChatboxEntityId = CreateChatbox(mWorld);
        
        WriteTextAtTextboxCoords(mainChatboxEntityId, "Use item on which", 1, 2, mWorld);
        WriteTextAtTextboxCoords(mainChatboxEntityId, "POK^MON?", 1, 4, mWorld);
        
        auto healthToDisplay = pokemonSelectionViewComponent.mFloatPokemonHealth;
        
        if (static_cast<int>(pokemonSelectionViewComponent.mFloatPokemonHealth) > selectedPokemon.mMaxHp)
        {
            const int hpRecovered = selectedPokemon.mMaxHp - selectedPokemon.mHp;
            selectedPokemon.mHp = selectedPokemon.mMaxHp;
            healthToDisplay = static_cast<float>(selectedPokemon.mHp);
            
            const auto itemResultChatboxEntityId = CreateChatbox(mWorld, ITEM_USAGE_RESULT_CHATBOX_POSITION);
            WriteTextAtTextboxCoords(itemResultChatboxEntityId, selectedPokemon.mName.GetString(), 1, 2, mWorld);
            WriteTextAtTextboxCoords(itemResultChatboxEntityId, "recovered by " + std::to_string(hpRecovered) + "!", 1, 4, mWorld);
        }
        else if (pokemonSelectionViewComponent.mHealthToRestoreCapacity <= 0.0f)
        {
            selectedPokemon.mHp = static_cast<int>(pokemonSelectionViewComponent.mFloatPokemonHealth);
            healthToDisplay = static_cast<float>(selectedPokemon.mHp);
            
            const auto itemResultChatboxEntityId = CreateChatbox(mWorld, ITEM_USAGE_RESULT_CHATBOX_POSITION);
            WriteTextAtTextboxCoords(itemResultChatboxEntityId, selectedPokemon.mName.GetString(), 1, 2, mWorld);
            WriteTextAtTextboxCoords(itemResultChatboxEntityId, "recovered by " + selectedItemStats.mEffect.GetString().substr(6) + "!", 1, 4, mWorld);
        }
        
        mWorld.DestroyEntity(pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex][1]);
        pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex][1] = LoadAndCreatePokemonHealthBar
        (
            healthToDisplay / selectedPokemon.mMaxHp,
            false,
            mWorld,
            false,
            true,
            pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex
        );
    }
}

void PokemonSelectionViewFlowState::InvalidOperationFlow()
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& guiStateSingletonComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    if (guiStateSingletonComponent.mActiveTextboxesStack.size() == 2)
    {
        pokemonSelectionViewComponent.mOperationState = PokemonSelectionViewOperationState::NORMAL;
        pokemonSelectionViewComponent.mPokemonHasBeenSelected = false;

        if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::ITEM_USAGE)
        {
            DestroyPokemonSelectionView();

            if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
            {
                CreateChatbox(mWorld);
            }
            else
            {
                CreateOverworldMainMenuTextbox(mWorld, HasMilestone(milestones::RECEIVED_POKEDEX, mWorld), playerStateComponent.mPreviousMainMenuCursorRow);
            }

            CompleteAndTransitionTo<ItemMenuFlowState>();
        }
        else
        {
            // Destroy pokemon attributes textbox
            DestroyActiveTextbox(mWorld);

            // Recreate view
            CreatePokemonSelectionViewMainTextbox();
            CreatePokemonStatsInvisibleTextbox();
        }
    }
}

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
                pokemonSelectionViewComponent.mOperationState = PokemonSelectionViewOperationState::INDEX_SWAP_FLOW;                
                PokemonRosterIndexSwapFlow();
            }
            else
            {
                TrySwitchPokemon();
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
        SoundService::GetInstance().PlaySfx(TEXTBOX_CLICK_SFX_NAME);
        
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
        auto& renderableComponent        = mWorld.GetComponent<RenderableComponent>(pokemonSpriteEntityId);

        animationTimerComponent.mAnimationTimer->Reset();
        animationTimerComponent.mAnimationTimer->Pause();
        renderableComponent.mActiveMeshIndex = 0;

        pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex = pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex;
        
        if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::ITEM_USAGE)
        {
            TryUseItem();
        }
        else if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::ENCOUNTER_AFTER_POKEMON_FAINTED)
        {
            TrySwitchPokemon();
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
        
        SoundService::GetInstance().PlaySfx(TEXTBOX_CLICK_SFX_NAME);
        
        if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::ITEM_USAGE)
        {
            DestroyPokemonSelectionView();

            if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
            {                
                CreateChatbox(mWorld);
            }
            else
            {
                DestroyActiveTextbox(mWorld);
                CreateOverworldMainMenuTextbox(mWorld, HasMilestone(milestones::RECEIVED_POKEDEX, mWorld), playerStateComponent.mPreviousMainMenuCursorRow);
            }

            CompleteAndTransitionTo<ItemMenuFlowState>();
        }
        else if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::OVERWORLD)
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
        auto& previousPokemonUnderCursorRenderableComponent     = mWorld.GetComponent<RenderableComponent>(previousPokemonUnderCursorSpriteEntityId);

        previousPokemonUnderCursorAnimationTimerComponent.mAnimationTimer->Reset();
        previousPokemonUnderCursorAnimationTimerComponent.mAnimationTimer->Pause();
        previousPokemonUnderCursorRenderableComponent.mActiveMeshIndex = 0;

        if (currentPokemonUnderCursor.mHp > 0)
        {
            const auto pokemonSpriteEntityId = pokemonSelectionViewComponent.mPokemonSpriteEntityIds[cursorComponent.mCursorRow][0];
            auto& animationTimerComponent    = mWorld.GetComponent<AnimationTimerComponent>(pokemonSpriteEntityId);            
            animationTimerComponent.mAnimationTimer->Resume();
        }

        pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex = cursorComponent.mCursorRow;
    }    
}

void PokemonSelectionViewFlowState::PokemonSelectionViewIndexSwapFlow(const float dt)
{
    const auto& inputStateComponent     = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();
    auto& playerStateComponent          = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    if (pokemonSelectionViewComponent.mSwapAnimationStep > 0)
    {
        pokemonSelectionViewComponent.mSwapIndexAnimationTimer->Update(dt);
        if (pokemonSelectionViewComponent.mSwapIndexAnimationTimer->HasTicked())
        {
            pokemonSelectionViewComponent.mSwapIndexAnimationTimer->Reset();
            if (pokemonSelectionViewComponent.mSwapAnimationStep++ > 2)
            {
                pokemonSelectionViewComponent.mSwapAnimationStep = 0;

                if (pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex != pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex)
                {
                    auto originallySelectedPokemon = std::move(playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex]);
                    auto pokemonToSwapWith = std::move(playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex]);

                    playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex] = std::move(pokemonToSwapWith);
                    playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex] = std::move(originallySelectedPokemon);
                }
                
                // Destroy invisible textbox
                DestroyActiveTextbox(mWorld);

                // Destroy which pokemon to swap chatbox
                DestroyActiveTextbox(mWorld);

                CreateIndividualPokemonSprites();

                CreatePokemonSelectionViewMainTextbox();

                CreatePokemonStatsInvisibleTextbox();                
            }
            else
            {
                if (pokemonSelectionViewComponent.mSwapAnimationStep == 2)
                {
                    const auto pokemonSelectionViewInfoTextboxEntityId = GetActiveTextboxEntityId(mWorld);
                    
                    SoundService::GetInstance().PlaySfx(POKEMON_SWAP_SFX_NAME);

                    mWorld.RemoveComponent<CursorComponent>(pokemonSelectionViewInfoTextboxEntityId);
                    DeleteTextAtTextboxRow(pokemonSelectionViewInfoTextboxEntityId, pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex * 2, mWorld);
                    DeleteTextAtTextboxRow(pokemonSelectionViewInfoTextboxEntityId, pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex * 2 + 1, mWorld);
                    
                    mWorld.DestroyEntity(pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex][0]);
                    mWorld.DestroyEntity(pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex][1]);
                    mWorld.DestroyEntity(pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex][2]);

                    pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex][0] = ecs::NULL_ENTITY_ID;
                    pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex][1] = ecs::NULL_ENTITY_ID;
                    pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex][2] = ecs::NULL_ENTITY_ID;
                    
                }
                else if (pokemonSelectionViewComponent.mSwapAnimationStep == 3)
                {
                    // If a self-swap is requested, don't actually erase for a second time the pokemon's data in the selection view
                    if (pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex == pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex)
                    {
                        return;
                    }

                    const auto pokemonSelectionViewInfoTextboxEntityId = GetActiveTextboxEntityId(mWorld);
                    
                    SoundService::GetInstance().PlaySfx(POKEMON_SWAP_SFX_NAME);

                    DeleteTextAtTextboxRow(pokemonSelectionViewInfoTextboxEntityId, pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex * 2, mWorld);
                    DeleteTextAtTextboxRow(pokemonSelectionViewInfoTextboxEntityId, pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex * 2 + 1, mWorld);
                                        
                    mWorld.DestroyEntity(pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex][0]);
                    mWorld.DestroyEntity(pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex][1]);
                    mWorld.DestroyEntity(pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex][2]);

                    pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex][0] = ecs::NULL_ENTITY_ID;
                    pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex][1] = ecs::NULL_ENTITY_ID;
                    pokemonSelectionViewComponent.mPokemonSpriteEntityIds[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex][2] = ecs::NULL_ENTITY_ID;
                }
            }
        }
    }
    else
    {
        if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
        {
            pokemonSelectionViewComponent.mPokemonHasBeenSelected  = false;
            pokemonSelectionViewComponent.mSwapAnimationStep       = 1;
            pokemonSelectionViewComponent.mSwapIndexAnimationTimer = std::make_unique<Timer>(INDEX_SWAP_ENTRY_DELAY);
            pokemonSelectionViewComponent.mSwapIndexAnimationTimer->SetTimerValue(0.0f);
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            SoundService::GetInstance().PlaySfx(TEXTBOX_CLICK_SFX_NAME);
            
            pokemonSelectionViewComponent.mOperationState = PokemonSelectionViewOperationState::NORMAL;            
            pokemonSelectionViewComponent.mPokemonHasBeenSelected = false;
            
            // Destroy invisible textbox
            DestroyActiveTextbox(mWorld);

            // Destroy which pokemon to swap chatbox
            DestroyActiveTextbox(mWorld);                       
            
            CreatePokemonSelectionViewMainTextbox();
            
            CreatePokemonStatsInvisibleTextbox();
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
            auto& previousPokemonUnderCursorRenderableComponent     = mWorld.GetComponent<RenderableComponent>(previousPokemonUnderCursorSpriteEntityId);

            previousPokemonUnderCursorAnimationTimerComponent.mAnimationTimer->Reset();
            previousPokemonUnderCursorAnimationTimerComponent.mAnimationTimer->Pause();
            previousPokemonUnderCursorRenderableComponent.mActiveMeshIndex = 0;

            if (currentPokemonUnderCursor.mHp > 0)
            {
                const auto pokemonSpriteEntityId = pokemonSelectionViewComponent.mPokemonSpriteEntityIds[cursorComponent.mCursorRow][0];
                auto& animationTimerComponent    = mWorld.GetComponent<AnimationTimerComponent>(pokemonSpriteEntityId);
                animationTimerComponent.mAnimationTimer->Resume();
            }
            
            pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex = cursorComponent.mCursorRow;
            
            if
            (
                 pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex !=
                 pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex
            )
            {
                const auto pokemonSelectionTextboxEntityId = GetActiveTextboxEntityId(mWorld);
                
                WriteCharAtTextboxCoords
                (
                    pokemonSelectionTextboxEntityId,
                    '{',
                    cursorComponent.mCursorDisplayHorizontalTileOffset + cursorComponent.mCursorDisplayHorizontalTileIncrements * pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex,
                    cursorComponent.mCursorDisplayVerticalTileOffset + cursorComponent.mCursorDisplayVerticalTileIncrements * pokemonSelectionViewComponent.mIndexSwapOriginPokemonCursorIndex,
                    mWorld
                );
            }
        }
    }
}

void PokemonSelectionViewFlowState::DisplayPokemonDetailedStatsFlow()
{
    // Destroy pokemon selected textbox
    DestroyActiveTextbox(mWorld);

    DestroyPokemonSelectionView();

    mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>().mPokemonHasBeenSelected = false;
    mWorld.GetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>().mSourceCreatorFlow = PokemonStatsDisplayViewCreationSourceType::POKEMON_SELECTION_VIEW;
    CompleteAndTransitionTo<PokemonStatsDisplayViewFlowState>();
}

void PokemonSelectionViewFlowState::TryUseItem()
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& playerStateComponent    = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& pokemonSelectionViewComponent = mWorld.GetSingletonComponent<PokemonSelectionViewStateSingletonComponent>();

    // Get selected item stats
    const auto& itemBagEntry      = playerStateComponent.mPlayerBag.at(playerStateComponent.mPreviousItemMenuItemOffset + playerStateComponent.mPreviousItemMenuCursorRow);
    const auto itemName           = itemBagEntry.mItemName;
    const auto& selectedItemStats = GetItemStats(itemBagEntry.mItemName, mWorld);

    // Get selected pokemons
    auto& selectedPokemon = *playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex];

    if (StringStartsWith(selectedItemStats.mEffect.GetString(), "POTION"))
    {
        if (selectedPokemon.mHp <= 0 || selectedPokemon.mHp == selectedPokemon.mMaxHp)
        {
            pokemonSelectionViewComponent.mOperationState = PokemonSelectionViewOperationState::INVALID_OPERATION;

            // Destroy pokemon stats textbox
            DestroyActiveTextbox(mWorld);

            // Destroy "Use item on which pokemon?" textbox
            DestroyActiveTextbox(mWorld);

            // Create placeholder chatbox for encounters
            if (encounterStateComponent.mActiveEncounterType != EncounterType::NONE)
            {
                CreateChatbox(mWorld);
            }

            // Recreate pokemon stats textbox
            CreatePokemonStatsInvisibleTextbox();

            const auto mainChatboxEntityId = CreateChatbox(mWorld);

            QueueDialogForChatbox(mainChatboxEntityId, "It won't have any#effect.#+END", mWorld);            
        }
        else
        {
            SoundService::GetInstance().PlaySfx(ITEM_HEAL_UP_SFX_NAME, true);

            pokemonSelectionViewComponent.mOperationState          = PokemonSelectionViewOperationState::HEALING_UP;
            pokemonSelectionViewComponent.mFloatPokemonHealth      = static_cast<float>(selectedPokemon.mHp);
            pokemonSelectionViewComponent.mHealthToRestoreCapacity = static_cast<float>(std::stoi(selectedItemStats.mEffect.GetString().substr(6)));
        }
    }
    
}

void PokemonSelectionViewFlowState::TrySwitchPokemon()
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

        pokemonSelectionViewComponent.mOperationState = PokemonSelectionViewOperationState::INVALID_OPERATION;        

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
        
        if (currentlyActivePokemon.mBindingOrWrappingOpponentCounter != -1)
        {
            currentlyActivePokemon.mBindingOrWrappingOpponentCounter = 0;
            currentlyActivePokemon.mBindingOrWrappingContinuationDamage = 0;
        }
        
        if 
        (
            std::count
            (
                encounterStateComponent.mPlayerPokemonIndicesEligibleForXp.cbegin(),
                encounterStateComponent.mPlayerPokemonIndicesEligibleForXp.cend(), 
                static_cast<size_t>(pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex)
            ) <= 0U
        )
        {
            encounterStateComponent.mPlayerPokemonIndicesEligibleForXp.push_back(pokemonSelectionViewComponent.mLastSelectedPokemonRosterIndex);
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

    pokemonSelectionViewEntities.mBackgroundEntityId = LoadAndCreatePokemonSelectionViewBackgroundCover
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

    pokemonSelectionViewStateComponent.mOperationState = PokemonSelectionViewOperationState::NORMAL;

    for (auto i = 0U; i < pokemonSpriteEntityIds.size(); ++i)
    {
        for (auto j = 0U; j < 3U; ++j)
        {
            if (pokemonSpriteEntityIds[i][j] != ecs::NULL_ENTITY_ID)
            {
                mWorld.DestroyEntity(pokemonSpriteEntityIds[i][j]);
            }
        }        
    }

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

    if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::ITEM_USAGE)
    {
        WriteTextAtTextboxCoords(mainChatboxEntityId, "Use item on which", 1, 2, mWorld);
        WriteTextAtTextboxCoords(mainChatboxEntityId, "POK^MON?", 1, 4, mWorld);
    }
    else if (pokemonSelectionViewComponent.mCreationSourceType == PokemonSelectionViewCreationSourceType::ENCOUNTER_AFTER_POKEMON_FAINTED)
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
        if
        (
            pokemonSelectionViewStateComponent.mOperationState == PokemonSelectionViewOperationState::HEALING_UP &&
            pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex == static_cast<int>(i)
        )
        {
            
            WriteTextAtTextboxCoords
            (
                pokemonSelectionViewTextboxEntityId,
                std::to_string(math::Min(static_cast<int>(pokemonSelectionViewStateComponent.mFloatPokemonHealth), pokemon.mMaxHp)) + "/",
                16 - static_cast<int>(std::to_string(pokemon.mHp).size()),
                i * 2 + 1,
                mWorld
            );            
        }
        else
        {
            WriteTextAtTextboxCoords
            (
                pokemonSelectionViewTextboxEntityId,
                std::to_string(pokemon.mHp) + "/",
                16 - static_cast<int>(std::to_string(pokemon.mHp).size()),
                i * 2 + 1,
                mWorld
            );
        }
        
        
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

    if 
    (
        pokemonSelectionViewStateComponent.mOperationState != PokemonSelectionViewOperationState::HEALING_UP &&
        pokemonSelectionViewStateComponent.mOperationState != PokemonSelectionViewOperationState::INVALID_OPERATION &&
        playerStateComponent.mPlayerPokemonRoster[pokemonSelectionViewStateComponent.mLastSelectedPokemonRosterIndex]->mHp > 0
    )
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

void PokemonSelectionViewFlowState::RefreshPlayerPokemonDisplayStats() const
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& activePlayerPokemon = *playerStateComponent.mPlayerPokemonRoster[encounterStateComponent.mActivePlayerPokemonRosterIndex];

    mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mPlayerPokemonHealthBarEntityId);

    encounterStateComponent.mViewObjects.mPlayerPokemonHealthBarEntityId = LoadAndCreatePokemonHealthBar
    (
        static_cast<float>(activePlayerPokemon.mHp) / activePlayerPokemon.mMaxHp,
        false,
        mWorld
    );

    // Write player's pokemon new current hp
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 1, 3, mWorld);
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 2, 3, mWorld);
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 3, 3, mWorld);

    WriteTextAtTextboxCoords
    (
        encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
        std::to_string(activePlayerPokemon.mHp) + "/",
        4 - static_cast<int>(std::to_string(activePlayerPokemon.mHp).size()),
        3,
        mWorld
    );

    // Write player's pokemon new max hp
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 5, 3, mWorld);
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 6, 3, mWorld);
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 7, 3, mWorld);

    WriteTextAtTextboxCoords
    (
        encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
        std::to_string(activePlayerPokemon.mMaxHp),
        8 - static_cast<int>(std::to_string(activePlayerPokemon.mMaxHp).size()),
        3,
        mWorld
    );

    // Write player's pokemon new level
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 4, 1, mWorld);
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 5, 1, mWorld);
    DeleteCharAtTextboxCoords(encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId, 6, 1, mWorld);

    if (activePlayerPokemon.mStatus == PokemonStatus::NORMAL || activePlayerPokemon.mStatus == PokemonStatus::CONFUSED)
    {
        WriteTextAtTextboxCoords
        (
            encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
            "=" + std::to_string(activePlayerPokemon.mLevel),
            4,
            1,
            mWorld
        );
    }
    else
    {
        WriteTextAtTextboxCoords
        (
            encounterStateComponent.mViewObjects.mPlayerPokemonInfoTextboxEntityId,
            GetFormattedPokemonStatus(activePlayerPokemon.mHp, activePlayerPokemon.mStatus),
            4,
            1,
            mWorld
        );
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
