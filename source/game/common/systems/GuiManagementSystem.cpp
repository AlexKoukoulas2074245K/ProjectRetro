//
//  GuiManagementSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 07/05/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "GuiManagementSystem.h"
#include "../GameConstants.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/TextboxComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../common/utils/StringUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../resources/DataFileResource.h"
#include "../../resources/MeshUtils.h"
#include "../../resources/ResourceLoadingService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

float GuiManagementSystem::GUI_TILE_DEFAULT_SIZE            = 0.11f;
float GuiManagementSystem::CHATBOX_BLINKING_CURSOR_COOLDOWN = 0.7f;
float GuiManagementSystem::CHATBOX_SCROLL_ANIM_COOLDOWN     = 0.1f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

GuiManagementSystem::GuiManagementSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<TextboxComponent>();
    InitializeGuiState();
}

void GuiManagementSystem::VUpdateAssociatedComponents(const float dt) const
{
    auto& inputStateComponent        = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& activeEntities       = mWorld.GetActiveEntities();
    const auto activeTextboxEntityId = GetActiveTextboxEntityId(mWorld);

    for (const auto& entityId: activeEntities)
    {
        if (ShouldProcessEntity(entityId) && entityId == activeTextboxEntityId)
        {
            auto& textboxComponent = mWorld.GetComponent<TextboxComponent>(entityId);
            
            switch (textboxComponent.mTextboxType)
            {
                case TextboxType::CHATBOX: UpdateChatbox(entityId, dt); break;
                case TextboxType::CURSORED_TEXTBOX: UpdateCursoredTextbox(entityId); break;
                case TextboxType::CURSORED_BARE_TEXTBOX:
                {
                    // We sanity check incase the cursor component has been externally removed
                    // from the textbox
                    if (mWorld.HasComponent<CursorComponent>(entityId))
                    {
                        UpdateCursoredTextbox(entityId);
                    } break;
                }
                case TextboxType::BARE_TEXTBOX: break;
                case TextboxType::GENERIC_TEXTBOX: break;
            }
            
            inputStateComponent.mHasBeenConsumed = true;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void GuiManagementSystem::InitializeGuiState() const
{
    ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_ATLASES_ROOT + "gui.png");

    auto guiStateSingletonComponent                   = std::make_unique<GuiStateSingletonComponent>();
    guiStateSingletonComponent->mGlobalGuiTileWidth   = GUI_TILE_DEFAULT_SIZE/GAME_TILE_SIZE;
    guiStateSingletonComponent->mGlobalGuiTileHeight  = GUI_TILE_DEFAULT_SIZE/GAME_TILE_SIZE;
    guiStateSingletonComponent->mActiveChatboxTimer   = std::make_unique<Timer>(DEFAULT_CHATBOX_CHAR_COOLDOWN);
    guiStateSingletonComponent->mMoreItemsCursorTimer = std::make_unique<Timer>(MORE_ITEMS_CURSOR_TIMER_DELAY);
    
    PopulateFontEntities(*guiStateSingletonComponent);
    
    mWorld.SetSingletonComponent<GuiStateSingletonComponent>(std::move(guiStateSingletonComponent));
}

void GuiManagementSystem::PopulateFontEntities(GuiStateSingletonComponent& guiStateComponent) const
{
    const auto fontCoordsResourceId = ResourceLoadingService::GetInstance().LoadResource(ResourceLoadingService::RES_DATA_ROOT + "font_coords.dat");
    const auto& fontCoordsResource  = ResourceLoadingService::GetInstance().GetResource<DataFileResource>(fontCoordsResourceId);
    
    const auto fontCoordsSplitByNewline = StringSplit(fontCoordsResource.GetContents(), '\n');
    
    for (auto row = 0U; row < fontCoordsSplitByNewline.size(); ++row)
    {
        const auto fontCoordsLineSplitBySpace = StringSplit(fontCoordsSplitByNewline[row], ' ');
        for (auto col = 0U; col < fontCoordsLineSplitBySpace.size(); ++col)
        {
            const auto currentFontCharacter = fontCoordsLineSplitBySpace[col][0];
            guiStateComponent.mFontEntities[currentFontCharacter] = LoadMeshFromAtlasTexCoords
            (
                col,
                row,
                GUI_ATLAS_COLS,
                GUI_ATLAS_ROWS,
                false,
                GUI_COMPONENTS_MODEL_NAME
            );
        }
    }
    
    ResourceLoadingService::GetInstance().UnloadResource(fontCoordsResourceId);
}


void GuiManagementSystem::UpdateChatbox(const ecs::EntityId textboxEntityId, const float dt) const
{
    auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& guiStateComponent   = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& textboxComponent    = mWorld.GetComponent<TextboxComponent>(textboxEntityId);

    if (textboxComponent.mQueuedDialog.size() > 0)
    {
        if (DetectedFreeze(textboxEntityId))
        {
            StripFreezeStringFromQueuedText(textboxEntityId);
            guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::FROZEN;
        }

        switch (guiStateComponent.mActiveChatboxDisplayState)
        {
            case ChatboxDisplayState::NORMAL: UpdateChatboxNormal(textboxEntityId, dt); break;
            case ChatboxDisplayState::FILLED: UpdateChatboxFilled(textboxEntityId, dt); break;
            case ChatboxDisplayState::SCROLL_ANIM_PHASE_1: UpdateChatboxScrollAnim1(textboxEntityId, dt); break;
            case ChatboxDisplayState::SCROLL_ANIM_PHASE_2: UpdateChatboxScrollAnim2(textboxEntityId, dt); break;
            case ChatboxDisplayState::PARAGRAPH_END_DELAY: UpdateChatboxParagraphEndDelay(dt); break;
            case ChatboxDisplayState::FROZEN: break;
        }
    }
    else if (guiStateComponent.mActiveChatboxContentState == ChatboxContentEndState::DIALOG_END)
    {
        if
        (
            IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||        
            IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent)        
        )
        {
            if (DetectedItemReceivedText(textboxEntityId))
            {
                OnItemReceived(textboxEntityId);
            }
            
            DestroyActiveTextbox(mWorld);
            guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::NORMAL;
            guiStateComponent.mActiveChatboxContentState = ChatboxContentEndState::NORMAL;
        }
    }
}

void GuiManagementSystem::UpdateChatboxNormal(const ecs::EntityId textboxEntityId, const float dt) const
{
    auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& guiStateComponent   = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();    
    
    if
    (
        IsActionTypeKeyPressed(VirtualActionType::A_BUTTON, inputStateComponent) ||        
        IsActionTypeKeyPressed(VirtualActionType::B_BUTTON, inputStateComponent)        
    )
    {        
        guiStateComponent.mActiveChatboxTimer->Update(2.0f * dt);
    }
    else
    {
        guiStateComponent.mActiveChatboxTimer->Update(dt);
    }
    
    if (guiStateComponent.mActiveChatboxTimer->HasTicked())
    {
        auto& textboxComponent = mWorld.GetComponent<TextboxComponent>(textboxEntityId);
        
        const auto& queuedParagraph = textboxComponent.mQueuedDialog.front();
        const auto& queuedLine      = queuedParagraph.front();
        const auto& queuedCharacter = queuedLine.front();
        
        //  Skip timer for space characters
        if (queuedCharacter != ' ')
        {
            guiStateComponent.mActiveChatboxTimer->Reset();
        }
        
        // Find first empty spot in the chat box to insert new character (beginning from top row,
        // folloed by bottom
        auto emptyColQuery = GetFirstEmptyColumnInTextboxRow(textboxEntityId, 2, mWorld);
        if (emptyColQuery == 0)
        {
            emptyColQuery = GetFirstEmptyColumnInTextboxRow(textboxEntityId, 4, mWorld);
            if (emptyColQuery == 0)
            {
                guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::FILLED;
                guiStateComponent.mActiveChatboxTimer        = std::make_unique<Timer>(CHATBOX_BLINKING_CURSOR_COOLDOWN);
                
                if (guiStateComponent.mActiveChatboxContentState != ChatboxContentEndState::DIALOG_END)
                {
                    WriteCharAtTextboxCoords(textboxEntityId, '|', textboxComponent.mTextboxTileCols - 2, textboxComponent.mTextboxTileRows - 2, mWorld);
                }
            }
            else
            {
                WriteCharAtTextboxCoords(textboxEntityId, queuedCharacter, emptyColQuery, 4, mWorld);
                OnTextboxQueuedCharacterRemoval(textboxEntityId);
            }
            
        }
        else
        {
            WriteCharAtTextboxCoords(textboxEntityId, queuedCharacter, emptyColQuery, 2, mWorld);
            OnTextboxQueuedCharacterRemoval(textboxEntityId);
        }
        
    }
}

void GuiManagementSystem::UpdateChatboxFilled(const ecs::EntityId textboxEntityId, const float dt) const
{
    auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    auto& guiStateComponent   = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if
    (
        IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||        
        IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent)        
    )
    {        
        guiStateComponent.mActiveChatboxTimer = std::make_unique<Timer>(CHATBOX_SCROLL_ANIM_COOLDOWN);
        
        switch (guiStateComponent.mActiveChatboxContentState)
        {
            case ChatboxContentEndState::NORMAL:
            {
                guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::SCROLL_ANIM_PHASE_1;
                
                if (DetectedItemReceivedText(textboxEntityId))
                {
                    OnItemReceived(textboxEntityId);
                }
                else if (DetectedKillSwitch(textboxEntityId))
                {
                    DestroyActiveTextbox(mWorld);
                    guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::NORMAL;
                    guiStateComponent.mActiveChatboxContentState = ChatboxContentEndState::NORMAL;
                    return;
                }
                
            } break;
                
            case ChatboxContentEndState::PARAGRAPH_END:
            {
                if (DetectedItemReceivedText(textboxEntityId))
                {
                    OnItemReceived(textboxEntityId);
                }
                
                DeleteTextAtTextboxRow(textboxEntityId, 2, mWorld);
                DeleteTextAtTextboxRow(textboxEntityId, 4, mWorld);
                guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::PARAGRAPH_END_DELAY;
            } break;
            
            case ChatboxContentEndState::DIALOG_END: break;
        }
    }
    
    if (guiStateComponent.mActiveChatboxContentState == ChatboxContentEndState::DIALOG_END)
    {
        return;
    }
    
    guiStateComponent.mActiveChatboxTimer->Update(dt);
    if (guiStateComponent.mActiveChatboxTimer->HasTicked())
    {
        guiStateComponent.mActiveChatboxTimer->Reset();
        auto& textboxComponent = mWorld.GetComponent<TextboxComponent>(textboxEntityId);
        if
        (
            GetCharAtTextboxCoords
            (
                textboxEntityId,
                textboxComponent.mTextboxTileCols - 2,
                textboxComponent.mTextboxTileRows - 2,
                mWorld
            ) == '|'
        )
        {
            DeleteCharAtTextboxCoords
            (
                textboxEntityId,
                textboxComponent.mTextboxTileCols - 2,
                textboxComponent.mTextboxTileRows - 2,
                mWorld
            );
        }
        else
        {
            WriteCharAtTextboxCoords
            (
                textboxEntityId,
                '|',
                textboxComponent.mTextboxTileCols - 2,
                textboxComponent.mTextboxTileRows - 2,
                mWorld
            );
        }
    }
}

void GuiManagementSystem::UpdateChatboxScrollAnim1(const ecs::EntityId textboxEntityId, const float dt) const
{
    auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    guiStateComponent.mActiveChatboxTimer->Update(dt);
    if (guiStateComponent.mActiveChatboxTimer->HasTicked())
    {
        guiStateComponent.mActiveChatboxTimer->Reset();
        
        auto& textboxComponent  = mWorld.GetComponent<TextboxComponent>(textboxEntityId);
        auto& textboxContent    = textboxComponent.mTextContent;
        
        const auto row2 = GetTextboxRowContent(textboxEntityId, 2, mWorld);
        const auto row4 = GetTextboxRowContent(textboxEntityId, 4, mWorld);
        
        for (auto textboxCol = 1U; textboxCol < textboxContent[0].size() - 1; ++textboxCol)
        {
            WriteCharAtTextboxCoords(textboxEntityId, row2[textboxCol].mCharacter, textboxCol, 1U, mWorld);
        }
        
        for (auto textboxCol = 1U; textboxCol < textboxContent[0].size() - 2; ++textboxCol)
        {
            WriteCharAtTextboxCoords(textboxEntityId, row4[textboxCol].mCharacter, textboxCol, 3U, mWorld);
        }
        
        WriteCharAtTextboxCoords(textboxEntityId, ' ', textboxContent[0].size() - 2, 3U, mWorld);
        
        DeleteTextAtTextboxRow(textboxEntityId, 2, mWorld);
        DeleteTextAtTextboxRow(textboxEntityId, 4, mWorld);

        guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::SCROLL_ANIM_PHASE_2;
    }
}

void GuiManagementSystem::UpdateChatboxScrollAnim2(const ecs::EntityId textboxEntityId, const float dt) const
{
    auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    guiStateComponent.mActiveChatboxTimer->Update(dt);
    if (guiStateComponent.mActiveChatboxTimer->HasTicked())
    {
        guiStateComponent.mActiveChatboxTimer = std::make_unique<Timer>(DEFAULT_CHATBOX_CHAR_COOLDOWN);
        
        auto& textboxComponent  = mWorld.GetComponent<TextboxComponent>(textboxEntityId);
        auto& textboxContent    = textboxComponent.mTextContent;
        
        const auto row3 = GetTextboxRowContent(textboxEntityId, 3, mWorld);
        
        for (auto textboxCol = 1U; textboxCol < textboxContent[0].size() - 1; ++textboxCol)
        {
            WriteCharAtTextboxCoords(textboxEntityId, row3[textboxCol].mCharacter, textboxCol, 2U, mWorld);
        }
        
        DeleteTextAtTextboxRow(textboxEntityId, 1, mWorld);
        DeleteTextAtTextboxRow(textboxEntityId, 3, mWorld);
        
        guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::NORMAL;
    }
}

void GuiManagementSystem::UpdateChatboxParagraphEndDelay(const float dt) const
{
    auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    guiStateComponent.mActiveChatboxTimer->Update(dt);
    if (guiStateComponent.mActiveChatboxTimer->HasTicked())
    {
        guiStateComponent.mActiveChatboxTimer = std::make_unique<Timer>(DEFAULT_CHATBOX_CHAR_COOLDOWN);
        
        guiStateComponent.mActiveChatboxContentState = ChatboxContentEndState::NORMAL;
        guiStateComponent.mActiveChatboxDisplayState = ChatboxDisplayState::NORMAL;
    }
}

void GuiManagementSystem::OnTextboxQueuedCharacterRemoval(const ecs::EntityId textboxEntityId) const
{
    auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& textboxComponent  = mWorld.GetComponent<TextboxComponent>(textboxEntityId);
    auto& queuedParagraph   = textboxComponent.mQueuedDialog.front();
    auto& queuedLine        = queuedParagraph.front();
    
    queuedLine.pop();
    if (queuedLine.size() == 0)
    {
        queuedParagraph.pop();
        if (queuedParagraph.size() == 0)
        {
            guiStateComponent.mActiveChatboxContentState = ChatboxContentEndState::PARAGRAPH_END;
            textboxComponent.mQueuedDialog.pop();
            if (textboxComponent.mQueuedDialog.size() == 0)
            {
                guiStateComponent.mActiveChatboxContentState = ChatboxContentEndState::DIALOG_END;
            }
        }
    }
}

void GuiManagementSystem::OnItemReceived(const ecs::EntityId textboxEntityId) const
{
    auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto textboxFirstLineString = GetTextboxRowString(textboxEntityId, 4, mWorld);
    if (textboxFirstLineString[0] == '\0')
    {
        textboxFirstLineString = textboxFirstLineString.substr(1);
    }
    
    const auto itemNameSplitBySpace = StringSplit(textboxFirstLineString, ' ');
    const auto itemNameSplitByExclamationMark = StringSplit(itemNameSplitBySpace[0], '!');
    
    playerStateComponent.mPendingItemToBeAdded = StringId(itemNameSplitByExclamationMark[0]);
}

void GuiManagementSystem::UpdateCursoredTextbox(const ecs::EntityId textboxEntityId) const
{    
    auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    inputStateComponent.mHasBeenConsumed = true;
    
    if (IsActionTypeKeyTapped(VirtualActionType::LEFT_ARROW, inputStateComponent))
    {
        MoveTextboxCursor(textboxEntityId, Direction::WEST);
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::RIGHT_ARROW, inputStateComponent))
    {
        MoveTextboxCursor(textboxEntityId, Direction::EAST);
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::UP_ARROW, inputStateComponent))
    {
        MoveTextboxCursor(textboxEntityId, Direction::NORTH);
    }
    else if (IsActionTypeKeyTapped(VirtualActionType::DOWN_ARROW, inputStateComponent))
    {
        MoveTextboxCursor(textboxEntityId, Direction::SOUTH);
    }
    else
    {
        inputStateComponent.mHasBeenConsumed = false;
    }
}

bool GuiManagementSystem::DetectedKillSwitch(const ecs::EntityId textboxEntityId) const
{
    const auto& textboxComponent = mWorld.GetComponent<TextboxComponent>(textboxEntityId);
    const auto& queuedParagraph  = textboxComponent.mQueuedDialog.front();
    const auto& queuedLine       = queuedParagraph.front();
    auto queuedLineCopy          = queuedLine;
    
    if (queuedLineCopy.size() >= 4)
    {
        if (queuedLineCopy.front() == '+')
        {
            queuedLineCopy.pop();
            if (queuedLineCopy.front() == 'E')
            {
                queuedLineCopy.pop();
                if (queuedLineCopy.front() == 'N')
                {
                    queuedLineCopy.pop();
                    if (queuedLineCopy.front() == 'D')
                    {
                        return true;
                    }
                }
            }
        }        
    }
    
    return false;
}

bool GuiManagementSystem::DetectedFreeze(const ecs::EntityId textboxEntityId) const
{
    const auto& textboxComponent = mWorld.GetComponent<TextboxComponent>(textboxEntityId);

    if (textboxComponent.mQueuedDialog.size() <= 0 || textboxComponent.mQueuedDialog.front().size() <= 0) return false;

    const auto& queuedParagraph = textboxComponent.mQueuedDialog.front();    
    const auto& queuedLine      = queuedParagraph.front();
    auto queuedLineCopy         = queuedLine;
    
    if (queuedLineCopy.size() >= 7)
    {
        if (queuedLineCopy.front() == '+')
        {
            queuedLineCopy.pop();
            if (queuedLineCopy.front() == 'F')
            {
                queuedLineCopy.pop();
                if (queuedLineCopy.front() == 'R')
                {
                    queuedLineCopy.pop();
                    if (queuedLineCopy.front() == 'E')
                    {
                        queuedLineCopy.pop();
                        if (queuedLineCopy.front() == 'E')
                        {
                            queuedLineCopy.pop();
                            if (queuedLineCopy.front() == 'Z')
                            {
                                queuedLineCopy.pop();
                                if (queuedLineCopy.front() == 'E')
                                {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }        
    }
    
    return false;
}

bool GuiManagementSystem::DetectedItemReceivedText(const ecs::EntityId textboxEntityId) const
{
    auto textboxFirstLineString = GetTextboxRowString(textboxEntityId, 2, mWorld);
    if (textboxFirstLineString[0] == '\0')
    {
        textboxFirstLineString = textboxFirstLineString.substr(1);
    }
    
    if
    (
        StringStartsWith(textboxFirstLineString, "PLAYERNAME got") ||
        StringStartsWith(textboxFirstLineString, "PLAYERNAME found") ||
        StringStartsWith(textboxFirstLineString, "PLAYERNAME received")
    )
    {
        return true;
    }
    
    return false;
}

void GuiManagementSystem::StripFreezeStringFromQueuedText(const ecs::EntityId textboxEntityId) const
{
    auto& textboxComponent  = mWorld.GetComponent<TextboxComponent>(textboxEntityId);
    auto& currentQueuedLine = textboxComponent.mQueuedDialog.front().front();
    TextboxQueuedTextLine mStrippedText;
    
    for(;;)
    {
        if (currentQueuedLine.front() == '+')
        {
            break;
        }
        else
        {
            mStrippedText.push(currentQueuedLine.front());
            currentQueuedLine.pop();
        }
    }

    textboxComponent.mQueuedDialog.front().front() = mStrippedText;
}

void GuiManagementSystem::MoveTextboxCursor(const ecs::EntityId textboxEntityId, const Direction direction) const
{
    auto& cursorComponent = mWorld.GetComponent<CursorComponent>(textboxEntityId);
    
    DeleteCharAtTextboxCoords
    (
        textboxEntityId, 
        cursorComponent.mCursorDisplayHorizontalTileOffset + cursorComponent.mCursorDisplayHorizontalTileIncrements * cursorComponent.mCursorCol,
        cursorComponent.mCursorDisplayVerticalTileOffset + cursorComponent.mCursorDisplayVerticalTileIncrements * cursorComponent.mCursorRow,
        mWorld
    );

    switch (direction)
    {
        case Direction::EAST:  cursorComponent.mCursorCol++; break;
        case Direction::NORTH: cursorComponent.mCursorRow--; break;
        case Direction::SOUTH: cursorComponent.mCursorRow++; break;
        case Direction::WEST:  cursorComponent.mCursorCol--; break;
    }

    if (cursorComponent.mCursorCol >= cursorComponent.mCursorColCount)
    {
        cursorComponent.mCursorCol = cursorComponent.mWarp ? 0 : cursorComponent.mCursorColCount - 1;
    }
    else if (cursorComponent.mCursorCol < 0)
    {
        cursorComponent.mCursorCol = cursorComponent.mWarp ? cursorComponent.mCursorColCount - 1 : 0;
    }
    else if (cursorComponent.mCursorRow >= cursorComponent.mCursorRowCount)
    {
        cursorComponent.mCursorRow = cursorComponent.mWarp ? 0 : cursorComponent.mCursorRowCount - 1;
    }
    else if (cursorComponent.mCursorRow < 0)
    {
        cursorComponent.mCursorRow = cursorComponent.mWarp ? cursorComponent.mCursorRowCount - 1 : 0;
    }

    WriteCharAtTextboxCoords
    (
        textboxEntityId,
        '}',
        cursorComponent.mCursorDisplayHorizontalTileOffset + cursorComponent.mCursorDisplayHorizontalTileIncrements * cursorComponent.mCursorCol,
        cursorComponent.mCursorDisplayVerticalTileOffset + cursorComponent.mCursorDisplayVerticalTileIncrements * cursorComponent.mCursorRow,
        mWorld
    );
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
