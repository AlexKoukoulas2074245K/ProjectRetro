//
//  BallUsageResultTextEncounterFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 09/08/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BallUsageResultTextEncounterFlowState.h"
#include "TurnOverEncounterFlowState.h"
#include "PoisonTickCheckEncounterFlowState.h"
#include "PokedexPokemonEntryDisplayFlowState.h"
#include "PokemonNicknameQuestionTextEncounterFlowState.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../utils/PokedexUtils.h"
#include "../utils/PokemonUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../encounter/components/EncounterStateSingletonComponent.h"
#include "../../encounter/utils/EncounterSpriteUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string BallUsageResultTextEncounterFlowState::CAUGHT_POKEMON_SFX_NAME = "encounter/caught_pokemon";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

BallUsageResultTextEncounterFlowState::BallUsageResultTextEncounterFlowState(ecs::World& world)
    : BaseFlowState(world)
{
    DisplayCatchResultText();
}

void BallUsageResultTextEncounterFlowState::VUpdate(const float)
{
    if (SoundService::GetInstance().IsPlayingSfx()) return;
    
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    if (guiStateComponent.mActiveChatboxContentState == ChatboxContentEndState::PARAGRAPH_END && encounterStateComponent.mWasPokemonCaught)
    {
        if (SoundService::GetInstance().GetLastPlayedSfxName() != CAUGHT_POKEMON_SFX_NAME)
        {
            SoundService::GetInstance().PlaySfx(CAUGHT_POKEMON_SFX_NAME, true, true);
            return;
        }
    }
    if (guiStateComponent.mActiveTextboxesStack.size() == 1)
    {
        if (encounterStateComponent.mWasPokemonCaught)
        {
            if (GetPokedexEntryTypeForPokemon(encounterStateComponent.mOpponentPokemonRoster.at(0)->mName, mWorld) != PokedexEntryType::OWNED)
            {
                // Destroy all encounter sprites
                DestroyEncounterSprites(mWorld);

                // Destroy main chatbox
                DestroyActiveTextbox(mWorld);

                // Destroy Last Frame of Pokemon Caught animation
                if (encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId != ecs::NULL_ENTITY_ID)
                {
                    mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId);
                    encounterStateComponent.mViewObjects.mBattleAnimationFrameEntityId = ecs::NULL_ENTITY_ID;
                }

                auto& pokedexStateComponent = mWorld.GetSingletonComponent<PokedexStateSingletonComponent>();
                pokedexStateComponent.mSelectedPokemonName = encounterStateComponent.mOpponentPokemonRoster.front()->mName;
                CompleteAndTransitionTo<PokedexPokemonEntryDisplayFlowState>();
            }
            else
            {
                DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, mWorld);
                mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId);
                mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId);

                encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId = ecs::NULL_ENTITY_ID;
                encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId      = ecs::NULL_ENTITY_ID;
                encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId   = ecs::NULL_ENTITY_ID;

                CompleteAndTransitionTo<PokemonNicknameQuestionTextEncounterFlowState>();
            }
            
            ChangePokedexEntryForPokemon(encounterStateComponent.mOpponentPokemonRoster.front()->mName, PokedexEntryType::OWNED, mWorld);
        }
        else
        {            
            encounterStateComponent.mLastEncounterMainMenuActionSelected = MainMenuActionType::ITEM;            
            CompleteAndTransitionTo<PoisonTickCheckEncounterFlowState>();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void BallUsageResultTextEncounterFlowState::DisplayCatchResultText() const
{
    const auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();

    // Destroy ball usage chatbox
    DestroyActiveTextbox(mWorld);

    const auto mainChatboxEntityId = CreateChatbox(mWorld);
    std::string catchResultText = "";

    if (encounterStateComponent.mWasPokemonCaught)
    {
        catchResultText += "All right!#" + encounterStateComponent.mOpponentPokemonRoster.at(0)->mName.GetString() + " was#caught!#";
                
        if (GetPokedexEntryTypeForPokemon(encounterStateComponent.mOpponentPokemonRoster.at(0)->mName, mWorld) != PokedexEntryType::OWNED)
        {
            catchResultText += "@New POK^DEX data#will be added for#" + encounterStateComponent.mOpponentPokemonRoster.at(0)->mName.GetString() + "!#+END";
        }
        else
        {
            catchResultText += "+END";
        }        
    }
    else if (encounterStateComponent.mBallThrownShakeCount == -1)
    {
        catchResultText += "The trainer#blocked the BALL!#@Don't be a thief!# #+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 0)
    {
        catchResultText += "You missed the#POK^MON!#+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 1)
    {
        catchResultText += "Darn! The POK^MON#broke free!#+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 2)
    {
        catchResultText += "Aww! It appeared#to be caught!#+END";
    }
    else if (encounterStateComponent.mBallThrownShakeCount == 3)
    {
        catchResultText += "Shoot! It was so#close too!#+END";
    }

    QueueDialogForChatbox(mainChatboxEntityId, catchResultText, mWorld);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
