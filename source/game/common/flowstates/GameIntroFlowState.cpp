//
//  DarkenedOpponentsIntroEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "GameIntroFlowState.h"
#include "NameSelectionFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/NameSelectionStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/TransitionAnimationStateSingletonComponent.h"
#include "../../sound/SoundService.h"

#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const StringId GameIntroFlowState::PIKACHU_CRY_SFX_NAME = StringId("cries/025");

const TileCoords GameIntroFlowState::NAME_SELECTION_LIST_DIMENSIONS = TileCoords(11, 12);

const glm::vec3 GameIntroFlowState::SPRITE_POSITION_CENTER          = glm::vec3(0.0f, 0.2f, 0.1f);
const glm::vec3 GameIntroFlowState::SPRITE_POSITION_OUTSIDE_SCREEN  = glm::vec3(0.9f, 0.2f, 0.1f);
const glm::vec3 GameIntroFlowState::SPRITE_POSITION_SIDE            = glm::vec3(0.4f, 0.2f, 0.1f);
const glm::vec3 GameIntroFlowState::NAME_SELECTION_TEXTBOX_POSITION = glm::vec3(-0.308800131f, 0.320599973f, 0.0f);
const glm::vec3 GameIntroFlowState::SPRITE_SCALE                    = glm::vec3(0.49f, 0.49f, 1.0f);

const float GameIntroFlowState::SPRITE_ANIMATION_SPEED = 2.0f;
const float GameIntroFlowState::COLOR_FLIP_TIMER_DELAY = 0.18f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

GameIntroFlowState::GameIntroFlowState(ecs::World& world)
    : BaseFlowState(world)
    , mColorFlipTimer(COLOR_FLIP_TIMER_DELAY)
    , mActiveCharacterSpriteEntityId(ecs::NULL_ENTITY_ID)    
{    
    const auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();
    if (introStateComponent.mIntroState == IntroState::OAK_FADING_IN)
    {
        mActiveCharacterSpriteEntityId = CreateCharacterSprite(IntroCharacter::OAK);
        SetColorFlipProgressionStep(1);
    }    
}

void GameIntroFlowState::VUpdate(const float dt)
{
    const auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    switch (introStateComponent.mIntroState)
    {
        case IntroState::OAK_FADING_IN: UpdateOakFadingInState(dt); break;
        case IntroState::OAK_WELCOME_SPEECH: UpdateOakWelcomeSpeechState(dt); break;
        case IntroState::OAK_WELCOME_SPEECH_FADE_OUT: UpdateOakWelcomeSpeechFadeOutState(dt); break;
        case IntroState::PIKACHU_MOVING_IN: UpdatePikachuMovingInState(dt); break;
        case IntroState::PIKACHU_CRY_TRIGGER: UpdatePikachuCryTriggerState(dt); break;
        case IntroState::POKEMON_INTRO_SPEECH: UpdatePokemonIntroSpeechState(dt); break;
        case IntroState::POKEMON_INTRO_SPEECH_FADE_OUT: UpdatePokemonIntroSpeechFadeOutState(dt); break;
        case IntroState::PLAYER_MOVING_IN: UpdatePlayerMovingInState(dt); break;
        case IntroState::PLAYERS_NAME_QUESTION: UpdatePlayersNameQuestionState(dt); break;
        case IntroState::PLAYER_MOVING_TO_THE_SIDE: UpdatePlayerMovingToTheSideState(dt); break;
        case IntroState::PRE_DEFINED_PLAYER_NAME_LIST: UpdatePreDefinedPlayerNameListState(dt); break;
        case IntroState::PLAYER_MOVING_BACK_TO_CENTER: UpdatePlayerMovingBackToCenterState(dt); break;
        case IntroState::PLAYER_NAME_CONFIRMATION_SPEECH: UpdatePlayerNameConfirmationSpeechState(dt); break;
        case IntroState::PLAYER_NAME_CONFIRMATION_SPEEH_FADE_OUT: UpdatePlayerNameConfirmationSpeechFadeOutState(dt); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void GameIntroFlowState::UpdateOakFadingInState(const float dt)
{
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    mColorFlipTimer.Update(dt);
    if (mColorFlipTimer.HasTicked())
    {
        mColorFlipTimer.Reset();
        SetColorFlipProgressionStep(GetColorFlipProgressionStep() + 1);
        if (GetColorFlipProgressionStep() == 6)
        {
            SetColorFlipProgressionStep(0);

            QueueDialogForChatbox
            (
                CreateChatbox(mWorld),
                "Hello there!#Welcome to the#world of POK^MON!#@My name is OAK!#People call me#the POK^MON PROF!#+END", 
                mWorld
            );

            introStateComponent.mIntroState = IntroState::OAK_WELCOME_SPEECH;
        }
    }
}

void GameIntroFlowState::UpdateOakWelcomeSpeechState(const float)
{
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        const auto trailingChatboxEntityId = CreateChatbox(mWorld);
        WriteTextAtTextboxCoords(trailingChatboxEntityId, "People call me", 1, 2, mWorld);
        WriteTextAtTextboxCoords(trailingChatboxEntityId, "the POK^MON PROF!", 1, 4, mWorld);

        SetColorFlipProgressionStep(6);

        introStateComponent.mIntroState = IntroState::OAK_WELCOME_SPEECH_FADE_OUT;
    }
}

void GameIntroFlowState::UpdateOakWelcomeSpeechFadeOutState(const float dt)
{
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    mColorFlipTimer.Update(dt);
    if (mColorFlipTimer.HasTicked())
    {
        mColorFlipTimer.Reset();

        SetColorFlipProgressionStep(GetColorFlipProgressionStep() + 1);
        if (GetColorFlipProgressionStep() == 9)
        {
            SetColorFlipProgressionStep(0);
            DestroyActiveTextbox(mWorld);
            mWorld.DestroyEntity(mActiveCharacterSpriteEntityId);
            mActiveCharacterSpriteEntityId = CreateCharacterSprite(IntroCharacter::PIKACHU);
            introStateComponent.mIntroState = IntroState::PIKACHU_MOVING_IN;
        }
    }
}

void GameIntroFlowState::UpdatePikachuMovingInState(const float dt)
{
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    auto& pikachuTransformComponent = mWorld.GetComponent<TransformComponent>(mActiveCharacterSpriteEntityId);    
    pikachuTransformComponent.mPosition.x -= SPRITE_ANIMATION_SPEED * dt;    

    if (pikachuTransformComponent.mPosition.x < SPRITE_POSITION_CENTER.x)
    {
        pikachuTransformComponent.mPosition.x = SPRITE_POSITION_CENTER.x;

        QueueDialogForChatbox(CreateChatbox(mWorld), "This world is#inhabited by#creatures called#POK^MON!#@For some people,#POK^MON are#pets. Others use#them for fights.#@Myself...# #@I study POK^MON#as a profession.#+END", mWorld);
        introStateComponent.mIntroState = IntroState::PIKACHU_CRY_TRIGGER;
    }
}

void GameIntroFlowState::UpdatePikachuCryTriggerState(const float)
{
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    if (guiStateComponent.mActiveChatboxContentState == ChatboxContentEndState::PARAGRAPH_END)
    {
        if (SoundService::GetInstance().GetLastPlayedSfxName() != PIKACHU_CRY_SFX_NAME)
        {
            SoundService::GetInstance().PlaySfx(PIKACHU_CRY_SFX_NAME, true, true);
            introStateComponent.mIntroState = IntroState::POKEMON_INTRO_SPEECH;
        }
    }
}

void GameIntroFlowState::UpdatePokemonIntroSpeechState(const float)
{        
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        const auto trailingChatboxEntityId = CreateChatbox(mWorld);
        WriteTextAtTextboxCoords(trailingChatboxEntityId, "I study POK^MON", 1, 2, mWorld);
        WriteTextAtTextboxCoords(trailingChatboxEntityId, "as a profession.", 1, 4, mWorld);

        SetColorFlipProgressionStep(6);        

        introStateComponent.mIntroState = IntroState::POKEMON_INTRO_SPEECH_FADE_OUT;
    }
}

void GameIntroFlowState::UpdatePokemonIntroSpeechFadeOutState(const float dt)
{
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    mColorFlipTimer.Update(dt);
    if (mColorFlipTimer.HasTicked())
    {
        mColorFlipTimer.Reset();

        SetColorFlipProgressionStep(GetColorFlipProgressionStep() + 1);
        if (GetColorFlipProgressionStep() == 9)
        {
            SetColorFlipProgressionStep(0);
            DestroyActiveTextbox(mWorld);
            mWorld.DestroyEntity(mActiveCharacterSpriteEntityId);
            mActiveCharacterSpriteEntityId = CreateCharacterSprite(IntroCharacter::PLAYER);
            introStateComponent.mIntroState = IntroState::PLAYER_MOVING_IN;
        }
    }
}

void GameIntroFlowState::UpdatePlayerMovingInState(const float dt)
{
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    auto& playerTransformComponent = mWorld.GetComponent<TransformComponent>(mActiveCharacterSpriteEntityId);
    playerTransformComponent.mPosition.x -= SPRITE_ANIMATION_SPEED * dt;

    if (playerTransformComponent.mPosition.x < SPRITE_POSITION_CENTER.x)
    {
        playerTransformComponent.mPosition.x = SPRITE_POSITION_CENTER.x;

        QueueDialogForChatbox(CreateChatbox(mWorld), "First, what is#your name?#+END", mWorld);
        introStateComponent.mIntroState = IntroState::PLAYERS_NAME_QUESTION;
    }
}

void GameIntroFlowState::UpdatePlayersNameQuestionState(const float)
{
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        const auto trailingChatboxEntityId = CreateChatbox(mWorld);
        WriteTextAtTextboxCoords(trailingChatboxEntityId, "First, what is", 1, 2, mWorld);
        WriteTextAtTextboxCoords(trailingChatboxEntityId, "your name?", 1, 4, mWorld);

        introStateComponent.mIntroState = IntroState::PLAYER_MOVING_TO_THE_SIDE;
    }
}

void GameIntroFlowState::UpdatePlayerMovingToTheSideState(const float dt)
{
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();
    auto& playerTransformComponent = mWorld.GetComponent<TransformComponent>(mActiveCharacterSpriteEntityId);
    playerTransformComponent.mPosition.x += SPRITE_ANIMATION_SPEED * dt;

    if (playerTransformComponent.mPosition.x > SPRITE_POSITION_SIDE.x)
    {
        playerTransformComponent.mPosition.x = SPRITE_POSITION_SIDE.x;

        CreateNameSelectionList(true);

        introStateComponent.mIntroState = IntroState::PRE_DEFINED_PLAYER_NAME_LIST;
    }
}

void GameIntroFlowState::UpdatePreDefinedPlayerNameListState(const float)
{   
    const auto& inputStateComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& cursorComponent     = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));

    auto& playerStateComponent        = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();    
    auto& introStateComponent         = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();
    auto& nameSelectionStateComponent = mWorld.GetSingletonComponent<NameSelectionStateSingletonComponent>();

    const auto cursorRow = cursorComponent.mCursorRow;

    if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
    {
        if (cursorRow == 0)
        {
            // Destroy name selection list
            DestroyActiveTextbox(mWorld);

            // Destroy chatbox
            DestroyActiveTextbox(mWorld);

            mWorld.DestroyEntity(mActiveCharacterSpriteEntityId);

            introStateComponent.mIntroState = IntroState::PLAYER_NAME_CONFIRMATION_SPEECH;

            nameSelectionStateComponent.mNameSelectionMode = NameSelectionMode::PLAYER_NAME;
            CompleteAndTransitionTo<NameSelectionFlowState>();
        }
        else
        {
            playerStateComponent.mPlayerTrainerName = StringId(ExtractPredefinedNameFromList());
            DestroyActiveTextbox(mWorld);
            introStateComponent.mIntroState = IntroState::PLAYER_MOVING_BACK_TO_CENTER;
        }
    }
}

void GameIntroFlowState::UpdatePlayerMovingBackToCenterState(const float dt)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    auto& playerTransformComponent = mWorld.GetComponent<TransformComponent>(mActiveCharacterSpriteEntityId);
    playerTransformComponent.mPosition.x -= SPRITE_ANIMATION_SPEED * dt;

    if (playerTransformComponent.mPosition.x < SPRITE_POSITION_CENTER.x)
    {
        playerTransformComponent.mPosition.x = SPRITE_POSITION_CENTER.x;

        DestroyActiveTextbox(mWorld);
        QueueDialogForChatbox(CreateChatbox(mWorld), "Right! So your#name is " + playerStateComponent.mPlayerTrainerName.GetString() + "!#+END", mWorld);

        introStateComponent.mIntroState = IntroState::PLAYER_NAME_CONFIRMATION_SPEECH;
    }
}

void GameIntroFlowState::UpdatePlayerNameConfirmationSpeechState(const float)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    if (mActiveCharacterSpriteEntityId == ecs::NULL_ENTITY_ID)
    {
        mActiveCharacterSpriteEntityId = CreateCharacterSprite(IntroCharacter::PLAYER);
        auto& playerTransformComponent = mWorld.GetComponent<TransformComponent>(mActiveCharacterSpriteEntityId);
        playerTransformComponent.mPosition.x = SPRITE_POSITION_CENTER.x;

        QueueDialogForChatbox(CreateChatbox(mWorld), "Right! So your#name is " + playerStateComponent.mPlayerTrainerName.GetString() + "!#+END", mWorld);
    }
    
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        const auto trailingChatboxEntityId = CreateChatbox(mWorld);
        WriteTextAtTextboxCoords(trailingChatboxEntityId, "Right! So your", 1, 2, mWorld);
        WriteTextAtTextboxCoords(trailingChatboxEntityId, "name is " + playerStateComponent.mPlayerTrainerName.GetString(), 1, 4, mWorld);

        SetColorFlipProgressionStep(6);

        introStateComponent.mIntroState = IntroState::PLAYER_NAME_CONFIRMATION_SPEEH_FADE_OUT;
    }
}

void GameIntroFlowState::UpdatePlayerNameConfirmationSpeechFadeOutState(const float dt)
{
    //auto& introStateComponent = mWorld.GetSingletonComponent<GameIntroStateSingletonComponent>();

    mColorFlipTimer.Update(dt);
    if (mColorFlipTimer.HasTicked())
    {
        mColorFlipTimer.Reset();

        SetColorFlipProgressionStep(GetColorFlipProgressionStep() + 1);
        if (GetColorFlipProgressionStep() == 9)
        {
            SetColorFlipProgressionStep(0);
            DestroyActiveTextbox(mWorld);
            mWorld.DestroyEntity(mActiveCharacterSpriteEntityId);

            mActiveCharacterSpriteEntityId = CreateCharacterSprite(IntroCharacter::RIVAL);            
        }
    }
}

ecs::EntityId GameIntroFlowState::CreateCharacterSprite(const IntroCharacter introCharacter) const
{        
    static const std::unordered_map<IntroCharacter, TileCoords> sCharacterToAtlasCoords = 
    {
        { IntroCharacter::OAK, TileCoords(8,2)    },
        { IntroCharacter::PLAYER, TileCoords(9,2) },
        { IntroCharacter::RIVAL, TileCoords(4,4)  },
        { IntroCharacter::PIKACHU, TileCoords()   },
    };

    switch (introCharacter)
    {
        case IntroCharacter::OAK:                 
        case IntroCharacter::RIVAL:
        case IntroCharacter::PLAYER:        
        {
            const auto& characterAtlasCoords = sCharacterToAtlasCoords.at(introCharacter);
            const auto& characterPosition = 
                introCharacter == IntroCharacter::PLAYER ? 
                SPRITE_POSITION_OUTSIDE_SCREEN : 
                SPRITE_POSITION_CENTER;

            return LoadAndCreateTrainerSprite
            (
                characterAtlasCoords.mCol,
                characterAtlasCoords.mRow,
                characterPosition,
                SPRITE_SCALE,
                mWorld
            );
        } break;

        case IntroCharacter::PIKACHU:
        {
            return LoadAndCreatePokemonSprite
            (
                StringId("PIKACHU"),
                true,
                SPRITE_POSITION_OUTSIDE_SCREEN,
                SPRITE_SCALE,
                mWorld
            );
        }break;

        default: return ecs::NULL_ENTITY_ID;
    }       
}

void GameIntroFlowState::CreateNameSelectionList(const bool forPlayer) const
{
    const auto textboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::CURSORED_TEXTBOX,
        NAME_SELECTION_LIST_DIMENSIONS.mCol,
        NAME_SELECTION_LIST_DIMENSIONS.mRow,
        NAME_SELECTION_TEXTBOX_POSITION.x,
        NAME_SELECTION_TEXTBOX_POSITION.y,
        NAME_SELECTION_TEXTBOX_POSITION.z,
        mWorld
    );

    if (forPlayer)
    {
        WriteTextAtTextboxCoords(textboxEntityId, "NEW NAME", 2, 2, mWorld);
        WriteTextAtTextboxCoords(textboxEntityId, "YELLOW", 2, 4, mWorld);
        WriteTextAtTextboxCoords(textboxEntityId, "ASH", 2, 6, mWorld);
        WriteTextAtTextboxCoords(textboxEntityId, "JACK", 2, 8, mWorld);
    }
    else
    {
        WriteTextAtTextboxCoords(textboxEntityId, "NEW NAME", 2, 2, mWorld);
        WriteTextAtTextboxCoords(textboxEntityId, "BLUE", 2, 4, mWorld);
        WriteTextAtTextboxCoords(textboxEntityId, "GARY", 2, 6, mWorld);
        WriteTextAtTextboxCoords(textboxEntityId, "JOHN", 2, 8, mWorld);
    }

    auto cursorComponent = std::make_unique<CursorComponent>();

    cursorComponent->mCursorCol = 0;
    cursorComponent->mCursorRow = 0;

    cursorComponent->mCursorColCount = 1;
    cursorComponent->mCursorRowCount = 4;

    cursorComponent->mCursorDisplayHorizontalTileOffset     = 1;
    cursorComponent->mCursorDisplayVerticalTileOffset       = 2;
    cursorComponent->mCursorDisplayHorizontalTileIncrements = 0;
    cursorComponent->mCursorDisplayVerticalTileIncrements   = 2;

    WriteCharAtTextboxCoords
    (
        textboxEntityId,
        '}',
        cursorComponent->mCursorDisplayHorizontalTileOffset + cursorComponent->mCursorDisplayHorizontalTileIncrements * cursorComponent->mCursorCol,
        cursorComponent->mCursorDisplayVerticalTileOffset + cursorComponent->mCursorDisplayVerticalTileIncrements * cursorComponent->mCursorRow,
        mWorld
    );

    cursorComponent->mWarp = false;

    mWorld.AddComponent<CursorComponent>(textboxEntityId, std::move(cursorComponent));    
}

std::string GameIntroFlowState::ExtractPredefinedNameFromList() const
{
    const auto activeTextboxEntityId   = GetActiveTextboxEntityId(mWorld);
    const auto& activeTextboxComponent = mWorld.GetComponent<TextboxComponent>(activeTextboxEntityId);
    const auto& cursorComponent        = mWorld.GetComponent<CursorComponent>(activeTextboxEntityId);
    const auto textboxRow              = cursorComponent.mCursorRow * 2 + 2;
    auto textboxCol                    = 2;

    std::string extractedName;
    while (activeTextboxComponent.mTextContent[textboxRow][textboxCol].mEntityId != ecs::NULL_ENTITY_ID)
    {
        extractedName += activeTextboxComponent.mTextContent[textboxRow][textboxCol++].mCharacter;
    }

    return extractedName;
}

int GameIntroFlowState::GetColorFlipProgressionStep() const
{
    return mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>().mIntroFlipProgressionStep;
}

void GameIntroFlowState::SetColorFlipProgressionStep(const int progressionStep) const
{
    mWorld.GetSingletonComponent<TransitionAnimationStateSingletonComponent>().mIntroFlipProgressionStep = progressionStep;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

