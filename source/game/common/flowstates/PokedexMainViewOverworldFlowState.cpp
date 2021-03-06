//
//  PokedexMainViewOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 31/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PokedexMainViewOverworldFlowState.h"
#include "PokedexPokemonEntryDisplayFlowState.h"
#include "TooImportantToTossFlowState.h"
#include "TownMapOverworldFlowState.h"
#include "ItemUsageFlowState.h"
#include "MainMenuEncounterFlowState.h"
#include "MainMenuOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/ItemMenuStateComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/MathUtils.h"
#include "../utils/PokedexUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/ResourceLoadingService.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PokedexMainViewOverworldFlowState::POKEDEX_MAIN_VIEW_BACKGROUND_POSITION           = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 PokedexMainViewOverworldFlowState::POKEDEX_MAIN_VIEW_BACKGROUND_SCALE              = glm::vec3(2.2f, 2.2f, 1.0f);
const glm::vec3 PokedexMainViewOverworldFlowState::POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_POSITION = glm::vec3(0.552100182f, 0.0f, -0.1f);

const std::string PokedexMainViewOverworldFlowState::TEXTBOX_CLICK_SFX_NAME = "general/textbox_click";
const std::string PokedexMainViewOverworldFlowState::POKEDEX_MAIN_VIEW_SPRITE_MODEL_FILE_NAME = "pokedex_main_view_background_sprite.obj";
const std::string PokedexMainViewOverworldFlowState::POKEDEX_MAIN_VIEW_BACKGROUND_TEXTURE_FILE_NAME = "pokedex_main_view_background.png";

const int PokedexMainViewOverworldFlowState::POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_COLS = 4;
const int PokedexMainViewOverworldFlowState::POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_ROWS = 16;

const float PokedexMainViewOverworldFlowState::POKEDEX_RAPID_SCROLL_ENABLING_TIMER_DELAY = 0.2f;
const float PokedexMainViewOverworldFlowState::POKEDEX_RAPID_SCROLL_ADVANCE_TIMER_DELAY  = 0.1f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PokedexMainViewOverworldFlowState::PokedexMainViewOverworldFlowState(ecs::World& world)
    : BaseFlowState(world)
    , mMaxSeenOrOwnedPokemonId(GetMaxSeenOrOwnedPokemonId(mWorld))
    , mPokedexRapidScrollEnablingTimer(POKEDEX_RAPID_SCROLL_ENABLING_TIMER_DELAY)
    , mPokedexRapidScrollAdvanceTimer(POKEDEX_RAPID_SCROLL_ADVANCE_TIMER_DELAY)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();    
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        CreatePokedexMainViewBackground();
        CreatePokedexMainViewInvisibleListTextbox(mWorld);
        DisplayPokedexEntriesForCurrentOffset();        
    } 
    else
    {
        DestroyActiveTextbox(mWorld);
    }
}

void PokedexMainViewOverworldFlowState::VUpdate(const float dt)
{    
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // Pokedex main menu view is active
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        UpdateMainView(dt);
    }   
    // Pokedex selected entry view is active
    else if (guiStateComponent.mActiveTextboxesStack.size() == 3)
    {
        UpdateSelectionView(dt);
    }    
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void PokedexMainViewOverworldFlowState::UpdateMainView(const float dt)
{    
    const auto& inputStateComponent    = mWorld.GetSingletonComponent<InputStateSingletonComponent>();    
    const auto pokedexMainViewEntityId = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent        = mWorld.GetComponent<CursorComponent>(pokedexMainViewEntityId);
    auto& itemMenuStateComponent       = mWorld.GetComponent<ItemMenuStateComponent>(pokedexMainViewEntityId);

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {        
        mSelectedPokemonId = itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow + 1;        
        if (GetPokedexEntryType(mSelectedPokemonId, mWorld) != PokedexEntryType::LOCKED)
        {
            DoActionInPokedexMainViewPokemonList(PokedexMainViewListActionType::SELECT_ENTRY);
            return;
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        CancelPokedexMainView();
        return;
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent))
    {        
        mPokedexRapidScrollEnablingTimer.Reset();
        mPokedexRapidScrollAdvanceTimer.Reset();
        
        if (cursorComponent.mCursorRow == 0 && itemMenuStateComponent.mPreviousCursorRow == 0)
        {
            DoActionInPokedexMainViewPokemonList(PokedexMainViewListActionType::SCROLL_UP);
        }        
    }
    else if (IsActionTypeKeyPressed(VirtualActionType::UP_ARROW, inputStateComponent))
    {
        mPokedexRapidScrollEnablingTimer.Update(dt);
        if (mPokedexRapidScrollEnablingTimer.HasTicked())
        {          
            mPokedexRapidScrollAdvanceTimer.Update(dt);
            if (mPokedexRapidScrollAdvanceTimer.HasTicked())
            {
                mPokedexRapidScrollAdvanceTimer.Reset();
                if (cursorComponent.mCursorRow == 0 && itemMenuStateComponent.mPreviousCursorRow == 0)
                {                
                    DoActionInPokedexMainViewPokemonList(PokedexMainViewListActionType::SCROLL_UP);
                }
                else
                {
                    MoveTextboxCursorToDirection(GetActiveTextboxEntityId(mWorld), Direction::NORTH, mWorld);
                }
            }
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent))
    {    
        mPokedexRapidScrollEnablingTimer.Reset();
        mPokedexRapidScrollAdvanceTimer.Reset();
        
        if (cursorComponent.mCursorRow == 6 && itemMenuStateComponent.mPreviousCursorRow == 6)
        {
            DoActionInPokedexMainViewPokemonList(PokedexMainViewListActionType::SCROLL_DOWN);
        }        
    }
    else if (IsActionTypeKeyPressed(VirtualActionType::DOWN_ARROW, inputStateComponent))
    {
        mPokedexRapidScrollEnablingTimer.Update(dt);
        if (mPokedexRapidScrollEnablingTimer.HasTicked())
        {           
            mPokedexRapidScrollAdvanceTimer.Update(dt);
            if (mPokedexRapidScrollAdvanceTimer.HasTicked())
            {
                mPokedexRapidScrollAdvanceTimer.Reset();
                if (cursorComponent.mCursorRow == 6 && itemMenuStateComponent.mPreviousCursorRow == 6)
                {
                    DoActionInPokedexMainViewPokemonList(PokedexMainViewListActionType::SCROLL_DOWN);
                }
                else
                {
                    MoveTextboxCursorToDirection(GetActiveTextboxEntityId(mWorld), Direction::SOUTH, mWorld);
                }
            }
        }        
    }   
    else if (IsActionTypeKeyTapped(VirtualActionType::LEFT_ARROW, inputStateComponent))
    {
        mPokedexRapidScrollEnablingTimer.Reset();
        mPokedexRapidScrollAdvanceTimer.Reset();
        
        if (itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow >= 6)
        {
            DoActionInPokedexMainViewPokemonList(PokedexMainViewListActionType::PAGE_UP);
        }
    }
    else if (IsActionTypeKeyPressed(VirtualActionType::LEFT_ARROW, inputStateComponent))
    {
        mPokedexRapidScrollEnablingTimer.Update(dt);
        if (mPokedexRapidScrollEnablingTimer.HasTicked())
        {
            mPokedexRapidScrollAdvanceTimer.Update(dt);
            if (mPokedexRapidScrollAdvanceTimer.HasTicked())
            {
                mPokedexRapidScrollAdvanceTimer.Reset();
                if (itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow >= 6)
                {
                    DoActionInPokedexMainViewPokemonList(PokedexMainViewListActionType::PAGE_UP);
                }
            }
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::RIGHT_ARROW, inputStateComponent))
    {
        mPokedexRapidScrollEnablingTimer.Reset();
        mPokedexRapidScrollAdvanceTimer.Reset();
        
        if (itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow <= mMaxSeenOrOwnedPokemonId - 7)
        {
            DoActionInPokedexMainViewPokemonList(PokedexMainViewListActionType::PAGE_DOWN);
        }
    }
    else if (IsActionTypeKeyPressed(VirtualActionType::RIGHT_ARROW, inputStateComponent))
    {
        mPokedexRapidScrollEnablingTimer.Update(dt);
        if (mPokedexRapidScrollEnablingTimer.HasTicked())
        {
            mPokedexRapidScrollAdvanceTimer.Update(dt);
            if (mPokedexRapidScrollAdvanceTimer.HasTicked())
            {
                mPokedexRapidScrollAdvanceTimer.Reset();
                if (itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow <= mMaxSeenOrOwnedPokemonId - 7)
                {
                    DoActionInPokedexMainViewPokemonList(PokedexMainViewListActionType::PAGE_DOWN);
                }
            }
        }
    }

    SaveLastFramesCursorRow();
}

void PokedexMainViewOverworldFlowState::UpdateSelectionView(const float)
{
    const auto& inputStateComponent         = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto pokedexSelectionViewEntityId = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent             = mWorld.GetComponent<CursorComponent>(pokedexSelectionViewEntityId);
    const auto cursorRow                    = cursorComponent.mCursorRow;

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {       
        // DATA
        if (cursorRow == 0)
        {
            DestroyActiveTextbox(mWorld);
            CreateChatbox(mWorld);
            CompleteAndTransitionTo<PokedexPokemonEntryDisplayFlowState>();
        }
        // CRY
        else if (cursorRow == 1)
        {
            SoundService::GetInstance().PlaySfx("cries/" + GetFormattedPokemonIdString(mSelectedPokemonId));
        }
        // AREA
        else if (cursorRow == 2)
        {                  
            DestroyActiveTextbox(mWorld);
            CreateChatbox(mWorld);
            CompleteAndTransitionTo<TownMapOverworldFlowState>();
        }
        // PRNT
        else if (cursorRow == 3)
        {

        }
        // QUIT
        else if (cursorRow == 4)
        {
            DestroyActiveTextbox(mWorld);
            CancelPokedexMainView();
        }
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
    {
        SoundService::GetInstance().PlaySfx(TEXTBOX_CLICK_SFX_NAME);
        DestroyActiveTextbox(mWorld);
    }
}

void PokedexMainViewOverworldFlowState::CancelPokedexMainView()
{
    auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();

    SoundService::GetInstance().PlaySfx(TEXTBOX_CLICK_SFX_NAME);
    
    DestroyActiveTextbox(mWorld);
    DestroyGenericOrBareTextbox(pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId, mWorld);
    mWorld.DestroyEntity(pokedexStateComponent.mPokedexMainViewBackgroundEntityId);

    pokedexStateComponent.mPokedexMainViewBackgroundEntityId          = ecs::NULL_ENTITY_ID;
    pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId = ecs::NULL_ENTITY_ID;
    pokedexStateComponent.mSelectedPokemonName                        = StringId();

    CompleteAndTransitionTo<MainMenuOverworldFlowState>();
}

void PokedexMainViewOverworldFlowState::RedrawPokedexMainView() const
{    
    const auto pokedexMainMenuViewEntityId = GetActiveTextboxEntityId(mWorld);
    const auto& cursorComponent            = mWorld.GetComponent<CursorComponent>(pokedexMainMenuViewEntityId);
    auto& itemMenuStateComponent           = mWorld.GetComponent<ItemMenuStateComponent>(pokedexMainMenuViewEntityId);

    const auto cursorRow  = cursorComponent.mCursorRow;
    const auto itemOffset = itemMenuStateComponent.mItemMenuOffsetFromStart;

    DestroyActiveTextbox(mWorld);
    CreatePokedexMainViewInvisibleListTextbox(mWorld, cursorRow, itemOffset);
    DisplayPokedexEntriesForCurrentOffset();
}

void PokedexMainViewOverworldFlowState::DisplayPokedexEntriesForCurrentOffset() const
{
    const auto& pokedexMainViewEntityId = GetActiveTextboxEntityId(mWorld);
    const auto& itemMenuComponent       = mWorld.GetComponent<ItemMenuStateComponent>(pokedexMainViewEntityId);    
   
    auto cursorRowIndex = 0U;
    for
    (
        auto i = itemMenuComponent.mItemMenuOffsetFromStart;
        i < math::Min(itemMenuComponent.mItemMenuOffsetFromStart + 7, mMaxSeenOrOwnedPokemonId);
        ++i
    )
    {
        const auto formattedPokemonPokedexId = GetFormattedPokemonIdString(i + 1);
        const auto pokemonName               = GetPokemonNameFromPokedexId(i + 1, mWorld);
        const auto pokedexEntryType          = GetPokedexEntryType(pokemonName, mWorld);

        WriteTextAtTextboxCoords(pokedexMainViewEntityId, formattedPokemonPokedexId, 1, cursorRowIndex * 2, mWorld);            

        if (pokedexEntryType == PokedexEntryType::LOCKED)
        {
            WriteTextAtTextboxCoords(pokedexMainViewEntityId, "----------", 4, 1 + cursorRowIndex * 2, mWorld);        
        }
        else
        {
            if (pokedexEntryType == PokedexEntryType::OWNED)
            {
                WriteSpecialCharacterAtTextboxCoords(pokedexMainViewEntityId, SpecialCharacter::POKEDEX_CAUGHT_BALL, 3, 1 + cursorRowIndex * 2, mWorld);
            }
            WriteTextAtTextboxCoords(pokedexMainViewEntityId, pokemonName.GetString(), 4, 1 + cursorRowIndex * 2, mWorld);
        }
            
        cursorRowIndex++;
    }    
}

void PokedexMainViewOverworldFlowState::SaveLastFramesCursorRow() const
{
    const auto pokedexMainViewEntityId = GetActiveTextboxEntityId(mWorld);    
    const auto& cursorComponent        = mWorld.GetComponent<CursorComponent>(pokedexMainViewEntityId);
    auto& itemMenuStateComponent       = mWorld.GetComponent<ItemMenuStateComponent>(pokedexMainViewEntityId);

    itemMenuStateComponent.mPreviousCursorRow = cursorComponent.mCursorRow;
}

void PokedexMainViewOverworldFlowState::CreatePokedexMainViewBackground()
{
    auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
    pokedexStateComponent.mPokedexMainViewBackgroundEntityId = mWorld.CreateEntity();

    auto renderableComponent = std::make_unique<RenderableComponent>();

    const auto texturePath = ResourceLoadingService::RES_TEXTURES_ROOT + POKEDEX_MAIN_VIEW_BACKGROUND_TEXTURE_FILE_NAME;
    renderableComponent->mTextureResourceId = ResourceLoadingService::GetInstance().LoadResource(texturePath);
    renderableComponent->mActiveAnimationNameId = StringId("default");
    renderableComponent->mShaderNameId = StringId("gui");
    renderableComponent->mAffectedByPerspective = false;

    const auto modelPath = ResourceLoadingService::RES_MODELS_ROOT + POKEDEX_MAIN_VIEW_SPRITE_MODEL_FILE_NAME;
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    renderableComponent->mAnimationsToMeshes[StringId("default")].push_back(resourceLoadingService.LoadResource(modelPath));

    auto transformComponent = std::make_unique<TransformComponent>();
    transformComponent->mPosition = POKEDEX_MAIN_VIEW_BACKGROUND_POSITION;
    transformComponent->mScale    = POKEDEX_MAIN_VIEW_BACKGROUND_SCALE;

    mWorld.AddComponent<RenderableComponent>(pokedexStateComponent.mPokedexMainViewBackgroundEntityId, std::move(renderableComponent));
    mWorld.AddComponent<TransformComponent>(pokedexStateComponent.mPokedexMainViewBackgroundEntityId, std::move(transformComponent));

    pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::BARE_TEXTBOX, 
        POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_COLS, 
        POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_ROWS, 
        POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_POSITION.x,
        POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_POSITION.y,
        POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_POSITION.z,
        mWorld
    );

    const auto ownedPokemon = GetNumberOfPokemonWithPokedexEntryType(PokedexEntryType::OWNED, mWorld);
    const auto seenPokemon  = ownedPokemon + GetNumberOfPokemonWithPokedexEntryType(PokedexEntryType::SEEN, mWorld);

    const auto ownedPokemonString = std::to_string(ownedPokemon);
    const auto seenPokemonString  = std::to_string(seenPokemon);

    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId, "SEEN", 0, 0, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId, seenPokemonString, 3 - seenPokemonString.size(), 1, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId, "OWN", 0, 3, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId, ownedPokemonString, 3 - ownedPokemonString.size(), 4, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId, "DATA", 0, 7, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId, "CRY", 0, 9, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId, "AREA", 0, 11, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId, "PRNT", 0, 13, mWorld);
    WriteTextAtTextboxCoords(pokedexStateComponent.mPokedexSelectionOptionsBareTextboxEntityId, "QUIT", 0, 15, mWorld);

}

void PokedexMainViewOverworldFlowState::DoActionInPokedexMainViewPokemonList(const PokedexMainViewListActionType actionType) const
{
    const auto pokedexMainViewEntityId = GetActiveTextboxEntityId(mWorld);   
    auto& cursorComponent              = mWorld.GetComponent<CursorComponent>(pokedexMainViewEntityId);
    auto& itemMenuStateComponent       = mWorld.GetComponent<ItemMenuStateComponent>(pokedexMainViewEntityId);

    switch (actionType)
    {
        case PokedexMainViewListActionType::SCROLL_UP:
        {
            itemMenuStateComponent.mItemMenuOffsetFromStart--;
            if (itemMenuStateComponent.mItemMenuOffsetFromStart <= 0)
            {
                itemMenuStateComponent.mItemMenuOffsetFromStart = 0;
            }            
        } break;

        case PokedexMainViewListActionType::SCROLL_DOWN:
        {
            itemMenuStateComponent.mItemMenuOffsetFromStart++;
            if (itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow >= mMaxSeenOrOwnedPokemonId - 1)
            {
                itemMenuStateComponent.mItemMenuOffsetFromStart = mMaxSeenOrOwnedPokemonId - 1 - cursorComponent.mCursorRow;
            }            
        } break;

        case PokedexMainViewListActionType::PAGE_UP:
        {
            itemMenuStateComponent.mItemMenuOffsetFromStart -= 7;
            if (itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow < 6)
            {
                itemMenuStateComponent.mItemMenuOffsetFromStart = 6 - cursorComponent.mCursorRow;
            }
        } break;

        case PokedexMainViewListActionType::PAGE_DOWN:
        {
            itemMenuStateComponent.mItemMenuOffsetFromStart += 7;
            if (itemMenuStateComponent.mItemMenuOffsetFromStart + cursorComponent.mCursorRow > mMaxSeenOrOwnedPokemonId - 7)
            {
                itemMenuStateComponent.mItemMenuOffsetFromStart = mMaxSeenOrOwnedPokemonId - 7 - cursorComponent.mCursorRow;
            }

        } break;

        case PokedexMainViewListActionType::SELECT_ENTRY:
        {            
            CreatePokedexSelectionViewInvisibleTextbox(mWorld);         

            auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
            pokedexStateComponent.mSelectedPokemonName = GetPokemonNameFromPokedexId(mSelectedPokemonId, mWorld);

            return;            
        } break;
    }

    RedrawPokedexMainView();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
