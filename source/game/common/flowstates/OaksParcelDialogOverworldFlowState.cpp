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

const std::string OaksParcelDialogOverworldFlowState::GARY_MUSIC_NAME     = "gary_trainer";
const std::string OaksParcelDialogOverworldFlowState::OAKS_LAB_MUSIC_NAME = "oaks_lab";

const TileCoords OaksParcelDialogOverworldFlowState::GARY_ENTRANCE_COORDS   = TileCoords(7, 5);
const TileCoords OaksParcelDialogOverworldFlowState::GARY_OAK_SPEECH_COORDS = TileCoords(7, 10);
const TileCoords OaksParcelDialogOverworldFlowState::GARY_ATLAS_COORDS      = TileCoords(4, 15);
const TileCoords OaksParcelDialogOverworldFlowState::FIRST_POKEDEX_COORDS   = TileCoords(5, 11);
const TileCoords OaksParcelDialogOverworldFlowState::SECOND_POKEDEX_COORDS  = TileCoords(6, 11);

const int OaksParcelDialogOverworldFlowState::FIRST_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX  = 4;
const int OaksParcelDialogOverworldFlowState::SECOND_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX = 5;

const float OaksParcelDialogOverworldFlowState::POKEDEX_DISAPPEARING_DELAY_IN_SECONDS = 0.5f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OaksParcelDialogOverworldFlowState::OaksParcelDialogOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
	, mEventState(EventState::INTRO_DIALOG)
	, mGarySpriteEntityId(ecs::NULL_ENTITY_ID)
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
        case EventState::GARY_INTRO:                 UpdateGaryIntro(); break;
        case EventState::GARY_ENTRANCE_PATH:         UpdateGaryPath(true); break;
		case EventState::POKEDEX_DIALOG:             UpdatePokedexDialog(); break;
		case EventState::POKEDEX_DISAPPEARING_DELAY: UpdatePokedexDisappearingDelay(dt); break;
		case EventState::OAK_SPEECH:                 UpdateOakSpeech(); break;
		case EventState::GARY_SPEECH:                UpdateGarySpeech(); break;
		case EventState::GARY_EXIT_PATH:             UpdateGaryPath(false); break;
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
        SoundService::GetInstance().PlayMusic(GARY_MUSIC_NAME, false);
        mEventState = EventState::GARY_INTRO;
        
        QueueDialogForChatbox(CreateChatbox(mWorld), playerStateComponent.mRivalName.GetString() + ": Gramps!", mWorld);
    }
}

void OaksParcelDialogOverworldFlowState::UpdateGaryIntro()
{    
    if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
		CreateGarySprite();
		CreateGaryPath(true);
        mEventState = EventState::GARY_ENTRANCE_PATH;
    }
}

void OaksParcelDialogOverworldFlowState::UpdateGaryPath(const bool isEnteringScene)
{
	const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
	auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(mGarySpriteEntityId);

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
			DestroyOverworldNpcEntityAndEraseTileInfo(mGarySpriteEntityId, mWorld);
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
		auto& garyRenderableComponent = mWorld.GetComponent<RenderableComponent>(mGarySpriteEntityId);
		ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::EAST), garyRenderableComponent);

		QueueDialogForChatbox
		(
			CreateChatbox(mWorld),
			playerStateComponent.mRivalName.GetString() + ": Alright#Gramps! Leave it#all to me!#@" + 
			playerStateComponent.mPlayerTrainerName.GetString() + ", I hate to#say it, but I#don't need you!#@I know! I'll#borrow a TOWN MAP#from my sis!#@" + 
			"I'll tell her not#to lend you one,#" + playerStateComponent.mPlayerTrainerName.GetString() + "! Hahaha!",
			mWorld
		);

		mEventState = EventState::GARY_SPEECH;
	}
}

void OaksParcelDialogOverworldFlowState::UpdateGarySpeech()
{		
	if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
	{
		SoundService::GetInstance().PlayMusic(GARY_MUSIC_NAME, false);
		CreateGaryPath(false);
		mEventState = EventState::GARY_EXIT_PATH;
	}
}

void OaksParcelDialogOverworldFlowState::CreateGarySprite()
{
	const auto& activeLevelComponent = mWorld.GetSingletonComponent<ActiveLevelSingletonComponent>();
	auto& levelModelComponent        = mWorld.GetComponent<LevelModelComponent>(GetLevelIdFromNameId(activeLevelComponent.mActiveLevelNameId, mWorld));

	mGarySpriteEntityId = mWorld.CreateEntity();
	
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
	transformComponent->mPosition = TileCoordsToPosition(GARY_ENTRANCE_COORDS.mCol, GARY_ENTRANCE_COORDS.mRow);

	auto movementStateComponent            = std::make_unique<MovementStateComponent>();
	movementStateComponent->mCurrentCoords = GARY_ENTRANCE_COORDS;

	GetTile(GARY_ENTRANCE_COORDS.mCol, GARY_ENTRANCE_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierEntityId = mGarySpriteEntityId;
	GetTile(GARY_ENTRANCE_COORDS.mCol, GARY_ENTRANCE_COORDS.mRow, levelModelComponent.mLevelTilemap).mTileOccupierType     = TileOccupierType::NPC;

	auto renderableComponent = CreateRenderableComponentForSprite(CharacterSpriteData(CharacterMovementType::DYNAMIC, GARY_ATLAS_COORDS.mCol, GARY_ATLAS_COORDS.mRow));
	ChangeAnimationIfCurrentPlayingIsDifferent(GetDirectionAnimationName(Direction::NORTH), *renderableComponent);

	mWorld.AddComponent<AnimationTimerComponent>(mGarySpriteEntityId, std::move(animationTimerComponent));
	mWorld.AddComponent<TransformComponent>(mGarySpriteEntityId, std::move(transformComponent));
	mWorld.AddComponent<LevelResidentComponent>(mGarySpriteEntityId, std::move(levelResidentComponent));
	mWorld.AddComponent<NpcAiComponent>(mGarySpriteEntityId, std::move(aiComponent));
	mWorld.AddComponent<MovementStateComponent>(mGarySpriteEntityId, std::move(movementStateComponent));
	mWorld.AddComponent<DirectionComponent>(mGarySpriteEntityId, std::move(directionComponent));
	mWorld.AddComponent<RenderableComponent>(mGarySpriteEntityId, std::move(renderableComponent));
}

void OaksParcelDialogOverworldFlowState::CreateGaryPath(const bool isEnteringScene)
{
	auto& garyAiComponent = mWorld.GetComponent<NpcAiComponent>(mGarySpriteEntityId);
	garyAiComponent.mAiTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);

	if (isEnteringScene)
	{
		garyAiComponent.mScriptedPathTileCoords.emplace_back(GARY_OAK_SPEECH_COORDS.mCol, GARY_OAK_SPEECH_COORDS.mRow);
	}
	else
	{
		garyAiComponent.mScriptedPathTileCoords.emplace_back(GARY_ENTRANCE_COORDS.mCol, GARY_ENTRANCE_COORDS.mRow);
	}

	garyAiComponent.mScriptedPathIndex = 0;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
