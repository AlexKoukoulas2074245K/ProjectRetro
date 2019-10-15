//
//  OaksParcelDialogOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 13/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "OaksParcelDialogOverworldFlowState.h"
#include "../components/DirectionComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../components/TransformComponent.h"
#include "../utils/MilestoneUtils.h"
#include "../utils/TextboxUtils.h"
#include "../../overworld/utils/OverworldCharacterLoadingUtils.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../overworld/utils/LevelUtils.h"
#include "../../overworld/components/ActiveLevelSingletonComponent.h"
#include "../../rendering/components/AnimationTimerComponent.h"
#include "../../rendering/utils/AnimationUtils.h"
#include "../../sound/SoundService.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string OaksParcelDialogOverworldFlowState::RIVAL_MUSIC_NAME     = "rival_trainer";
const std::string OaksParcelDialogOverworldFlowState::OAKS_LAB_MUSIC_NAME  = "oaks_lab";

const TileCoords OaksParcelDialogOverworldFlowState::RIVAL_ENTRANCE_COORDS   = TileCoords(7, 5);
const TileCoords OaksParcelDialogOverworldFlowState::RIVAL_OAK_SPEECH_COORDS = TileCoords(7, 10);
const TileCoords OaksParcelDialogOverworldFlowState::RIVAL_ATLAS_COORDS      = TileCoords(4, 15);
const TileCoords OaksParcelDialogOverworldFlowState::FIRST_POKEDEX_COORDS    = TileCoords(5, 11);
const TileCoords OaksParcelDialogOverworldFlowState::SECOND_POKEDEX_COORDS   = TileCoords(6, 11);

const int OaksParcelDialogOverworldFlowState::FIRST_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX  = 4;
const int OaksParcelDialogOverworldFlowState::SECOND_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX = 5;

const float OaksParcelDialogOverworldFlowState::POKEDEX_DISAPPEARING_DELAY_IN_SECONDS = 0.5f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OaksParcelDialogOverworldFlowState::OaksParcelDialogOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
	, mEventState(EventState::INTRO_DIALOG)
	, mRivalSpriteEntityId(ecs::NULL_ENTITY_ID)
	, mPokedexDisappearingTimer(POKEDEX_DISAPPEARING_DELAY_IN_SECONDS)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
        
    QueueDialogForChatbox
    (
        CreateChatbox(mWorld),
        "OAK: Oh, " + playerStateComponent.mPlayerTrainerName.GetString() + "!# #@How is my old#" +
        "POK^MON?#@Well, it seems to#like you a lot.#@You must be#talented as a#POK^MON trainer!#@" +
        "What? You have#something for me?#@" + playerStateComponent.mPlayerTrainerName.GetString() + " delivered#OAK's PARCEL.#@" +
        "Ah! This is the#custom POK^ BALL#I ordered!#Thanks, " + playerStateComponent.mPlayerTrainerName.GetString() + "!#@" +
        "By the way, I must#ask you to do#something for me.",
        mWorld
    );    
}

void OaksParcelDialogOverworldFlowState::VUpdate(const float dt)
{
    switch (mEventState)
    {
        case EventState::INTRO_DIALOG:               UpdateIntroDialog(); break;
        case EventState::RIVAL_INTRO:                 UpdateRivalIntro(); break;
        case EventState::RIVAL_ENTRANCE_PATH:         UpdateRivalPath(true); break;
		case EventState::POKEDEX_DIALOG:             UpdatePokedexDialog(); break;
		case EventState::POKEDEX_DISAPPEARING_DELAY: UpdatePokedexDisappearingDelay(dt); break;
		case EventState::OAK_SPEECH:                 UpdateOakSpeech(); break;
		case EventState::RIVAL_SPEECH:                UpdateRivalSpeech(); break;
		case EventState::RIVAL_EXIT_PATH:             UpdateRivalPath(false); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void OaksParcelDialogOverworldFlowState::UpdateIntroDialog()
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();    
    
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        SoundService::GetInstance().PlayMusic(RIVAL_MUSIC_NAME, false);
        mEventState = EventState::RIVAL_INTRO;
        
        QueueDialogForChatbox(CreateChatbox(mWorld), playerStateComponent.mRivalName.GetString() + ": Gramps!", mWorld);
    }
}

void OaksParcelDialogOverworldFlowState::UpdateRivalIntro()
{    
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
		CreateRivalSprite();
		CreateRivalPath(true);
        mEventState = EventState::RIVAL_ENTRANCE_PATH;
    }
}

void OaksParcelDialogOverworldFlowState::UpdateRivalPath(const bool isEnteringScene)
{
	const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
	auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(mRivalSpriteEntityId);

	if (npcAiComponent.mScriptedPathIndex == -1)
	{
		SoundService::GetInstance().PlayMusic(OAKS_LAB_MUSIC_NAME, false);

		if (isEnteringScene)
		{
			QueueDialogForChatbox
			(
				CreateChatbox(mWorld),
				playerStateComponent.mRivalName.GetString() + ": Gramps,#my POK^MON has#grown stronger!#Check it out!#@" +
				"OAK: " + playerStateComponent.mRivalName.GetString() + ",#good timing!#@I needed to ask#both of you to do#something for me.#@" +
				"On the desk there#is my invention,#POK^DEX!#@It automatically#records data on#POK^MON you've#seen or caught!#@It's a hi-tech#encyclopedia!#@" +
				"OAK: " + playerStateComponent.mPlayerTrainerName.GetString() + " and#" + playerStateComponent.mRivalName.GetString() + "! Take#these with you!#@" +
				playerStateComponent.mPlayerTrainerName.GetString() + " got#POK^DEX from OAK!",
				mWorld
			);

			mEventState = EventState::POKEDEX_DIALOG;
		}
		else
		{									
			DestroyOverworldNpcEntityAndEraseTileInfo(mRivalSpriteEntityId, mWorld);
			CompleteOverworldFlow();
		}
	}
}

void OaksParcelDialogOverworldFlowState::UpdatePokedexDialog()
{	
	if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
	{				
		mWorld.DestroyEntity(FindEntityAtLevelCoords(FIRST_POKEDEX_COORDS, mWorld));
		mWorld.DestroyEntity(FindEntityAtLevelCoords(SECOND_POKEDEX_COORDS, mWorld));
		DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(FIRST_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX, mWorld), mWorld);
		DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(SECOND_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX, mWorld), mWorld);

		mEventState = EventState::POKEDEX_DISAPPEARING_DELAY;
	}
}

void OaksParcelDialogOverworldFlowState::UpdatePokedexDisappearingDelay(const float dt)
{
	mPokedexDisappearingTimer.Update(dt);
	if (mPokedexDisappearingTimer.HasTicked())
	{
		QueueDialogForChatbox
		(
			CreateChatbox(mWorld),
			"To make a complete#guide on all the#POK^MON in the#world...#@That was my dream!# #@But I'm too old!#I can't do it!#@" + std::string() + 
			"So, I want you two#to fulfill my#dream for me!#@Get moving, you#two!#@This is a great#undertaking in#POK^MON history!",
			mWorld
		);

		mEventState = EventState::OAK_SPEECH;
	}
}

void OaksParcelDialogOverworldFlowState::UpdateOakSpeech()
{
	const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();	

	if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
	{
		auto& rivalRenderableComponent = mWorld.GetComponent<RenderableComponent>(mRivalSpriteEntityId);
		ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::EAST), rivalRenderableComponent);

		QueueDialogForChatbox
		(
			CreateChatbox(mWorld),
			playerStateComponent.mRivalName.GetString() + ": Alright#Gramps! Leave it#all to me!#@" + 
			playerStateComponent.mPlayerTrainerName.GetString() + ", I hate to#say it, but I#don't need you!#@I know! I'll#borrow a TOWN MAP#from my sis!#@" + 
			"I'll tell her not#to lend you one,#" + playerStateComponent.mPlayerTrainerName.GetString() + "! Hahaha!",
			mWorld
		);

		mEventState = EventState::RIVAL_SPEECH;
	}
}

void OaksParcelDialogOverworldFlowState::UpdateRivalSpeech()
{		
	if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
	{
		SoundService::GetInstance().PlayMusic(RIVAL_MUSIC_NAME, false);
		CreateRivalPath(false);
		mEventState = EventState::RIVAL_EXIT_PATH;
	}
}

void OaksParcelDialogOverworldFlowState::CreateRivalSprite()
{
	const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
	auto& levelModelComponent        = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));

	mRivalSpriteEntityId = mWorld.CreateEntity();
	
	auto animationTimerComponent             = std::make_unique<AnimationTimerComponent>();
	animationTimerComponent->mAnimationTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);
	animationTimerComponent->mAnimationTimer->Pause();

	auto aiComponent = std::make_unique<NpcAiComponent>();
	aiComponent->mMovementType  = CharacterMovementType::STATIC;	
	aiComponent->mInitDirection = Direction::NORTH;

	auto directionComponent        = std::make_unique<DirectionComponent>();
	directionComponent->mDirection = Direction::NORTH;

	auto levelResidentComponent          = std::make_unique<LevelResidentComponent>();
	levelResidentComponent->mLevelNameId = activeLevelComponent.mActiveLevelNameId;

	auto transformComponent       = std::make_unique<TransformComponent>();
	transformComponent->mPosition = TileCoordsToPosition(RIVAL_ENTRANCE_COORDS.mCol, RIVAL_ENTRANCE_COORDS.mRow);

	auto movementStateComponent            = std::make_unique<MovementStateComponent>();
	movementStateComponent->mCurrentCoords = RIVAL_ENTRANCE_COORDS;

	GetTile(RIVAL_ENTRANCE_COORDS.mCol, RIVAL_ENTRANCE_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = mRivalSpriteEntityId;
	GetTile(RIVAL_ENTRANCE_COORDS.mCol, RIVAL_ENTRANCE_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NPC;

	auto renderableComponent = CreateRenderableComponentForSprite(CharacterSpriteData(CharacterMovementType::DYNAMIC, RIVAL_ATLAS_COORDS.mCol, RIVAL_ATLAS_COORDS.mRow));
	ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::NORTH), *renderableComponent);

	mWorld.AddComponent<AnimationTimerComponent>(mRivalSpriteEntityId, std::move(animationTimerComponent));
	mWorld.AddComponent<TransformComponent>(mRivalSpriteEntityId, std::move(transformComponent));
	mWorld.AddComponent<LevelResidentComponent>(mRivalSpriteEntityId, std::move(levelResidentComponent));
	mWorld.AddComponent<NpcAiComponent>(mRivalSpriteEntityId, std::move(aiComponent));
	mWorld.AddComponent<MovementStateComponent>(mRivalSpriteEntityId, std::move(movementStateComponent));
	mWorld.AddComponent<DirectionComponent>(mRivalSpriteEntityId, std::move(directionComponent));
	mWorld.AddComponent<RenderableComponent>(mRivalSpriteEntityId, std::move(renderableComponent));
}

void OaksParcelDialogOverworldFlowState::CreateRivalPath(const bool isEnteringScene)
{
	auto& rivalAiComponent = mWorld.GetComponent<NpcAiComponent>(mRivalSpriteEntityId);
	rivalAiComponent.mAiTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);

	if (isEnteringScene)
	{
		rivalAiComponent.mScriptedPathTileCoords.emplace_back(RIVAL_OAK_SPEECH_COORDS.mCol, RIVAL_OAK_SPEECH_COORDS.mRow);
	}
	else
	{
		rivalAiComponent.mScriptedPathTileCoords.emplace_back(RIVAL_ENTRANCE_COORDS.mCol, RIVAL_ENTRANCE_COORDS.mRow);
	}

	rivalAiComponent.mScriptedPathIndex = 0;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
