//
//  AwardLevelFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 18/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "AwardLevelFlowState.h"
#include "NewMovesCheckFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../input/utils/InputUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string AwardLevelFlowState::POKEMON_LEVEL_UP_SFX_NAME = "general/level_up_or_badge";

const glm::vec3 AwardLevelFlowState::POKEMON_STATS_DISPLAY_TEXTBOX_POSITION = glm::vec3(0.31f, 0.2f, -0.4f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

AwardLevelFlowState::AwardLevelFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];

    LevelUpStats(mWorld, activePlayerPokemon);
    
    RefreshPokemonStats();

    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    QueueDialogForChatbox
    (
        mainChatboxEntityId,
        activePlayerPokemon.mName.GetString() + " grew#to level " + std::to_string(activePlayerPokemon.mLevel) + "!+FREEZE",
        mWorld
    );
}

void AwardLevelFlowState::VUpdate(const float)
{    
    auto& soundService = SoundService::GetInstance();
    if (soundService.IsPlayingSfx()) return;

    const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (guiStateComponent.mActiveChatboxDisplayState == ChatboxDisplayState::FROZEN)
    {                
        if (soundService.IsPlayingSfx() == false && soundService.GetLastPlayedSfxName() != POKEMON_LEVEL_UP_SFX_NAME)
        {
            soundService.PlaySfx(POKEMON_LEVEL_UP_SFX_NAME);
            soundService.MuteMusic();
            return;
        }

        SoundService::GetInstance().UnmuteMusic();

        if (encounterStateComponent.mViewObjects.mLevelUpNewStatsTextboxEntityId == ecs::NULL_ENTITY_ID)
        {
            encounterStateComponent.mViewObjects.mLevelUpNewStatsTextboxEntityId = CreatePokemonMiniStatsDisplay
            (
                activePlayerPokemon,
                POKEMON_STATS_DISPLAY_TEXTBOX_POSITION,
                mWorld
            );
        }
        else if 
        (
            IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent) ||
            IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent)
        )
        {
            DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mLevelUpNewStatsTextboxEntityId, mWorld);
            encounterStateComponent.mViewObjects.mLevelUpNewStatsTextboxEntityId = ecs::NULL_ENTITY_ID;            
            CompleteAndTransitionTo<NewMovesCheckFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void AwardLevelFlowState::RefreshPokemonStats() const
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& activePlayerPokemon        = *playerStateComponent.mPlayerPokemonRoster[playerStateComponent.mLeveledUpPokemonRosterIndex];

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
