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

const float OaksParcelDialogOverworldFlowState::POKEDEX_DISAPPEARING_DELAY = 0.5f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

OaksParcelDialogOverworldFlowState::OaksParcelDialogOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
	, mEventState(EventState::INTRO_DIALOG)
	, mGarySpriteEntityId(ecs::NULL_ENTITY_ID)
	, mPokedexDisappearingTimer(POKEDEX_DISAPPEARING_DELAY)
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    
    DestroyActiveTextbox(mWorld);
    
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
        case EventState::INTRO_DIALOG: UpdateIntroDialog(); break;
        case EventState::GARY_INTRO:   UpdateGaryIntro(); break;
        case EventState::GARY_PATH:    UpdateGaryPath(); break;
		case EventState::POKEDEX_DIALOG: UpdatePokedexDialog(); break;
		case EventState::POKEDEX_DISAPPEARING_DELAY: UpdatePokedexDisappearingDelay(dt); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void OaksParcelDialogOverworldFlowState::UpdateIntroDialog()
{
    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
    const auto& guiStateComponent    = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
        SoundService::GetInstance().PlayMusic(GARY_MUSIC_NAME, false);
        mEventState = EventState::GARY_INTRO;
        
        QueueDialogForChatbox(CreateChatbox(mWorld), playerStateComponent.mRivalName.GetString() + ": Gramps!", mWorld);
    }
}

void OaksParcelDialogOverworldFlowState::UpdateGaryIntro()
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
    
    if (guiStateComponent.mActiveTextboxesStack.size() == 0)
    {
		CreateGarySprite();
		CreateGaryPath(true);
        mEventState = EventState::GARY_PATH;
    }
}

void OaksParcelDialogOverworldFlowState::UpdateGaryPath()
{
	const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
	auto& npcAiComponent = mWorld.GetComponent<NpcAiComponent>(mGarySpriteEntityId);

	if (npcAiComponent.mScriptedPathIndex == -1)
	{
		SoundService::GetInstance().PlayMusic(OAKS_LAB_MUSIC_NAME, false);

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
}

void OaksParcelDialogOverworldFlowState::UpdatePokedexDialog()
{
	const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();
	if (guiStateComponent.mActiveTextboxesStack.size() == 0)
	{
		const auto& pokedexEntityIds = GetPokedexSpriteEntityIds();
		(void)pokedexEntityIds;
		mEventState = EventState::POKEDEX_DISAPPEARING_DELAY;
	}
}

void OaksParcelDialogOverworldFlowState::UpdatePokedexDisappearingDelay(const float dt)
{
	mPokedexDisappearingTimer.Update(dt);
	if (mPokedexDisappearingTimer.HasTicked())
	{

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

void OaksParcelDialogOverworldFlowState::CreateGaryPath(const bool)
{
	auto& garyAiComponent = mWorld.GetComponent<NpcAiComponent>(mGarySpriteEntityId);
	garyAiComponent.mAiTimer = std::make_unique<Timer>(CHARACTER_ANIMATION_FRAME_TIME);
	garyAiComponent.mScriptedPathTileCoords.emplace_back(GARY_OAK_SPEECH_COORDS.mCol, GARY_OAK_SPEECH_COORDS.mRow);	
	garyAiComponent.mScriptedPathIndex = 0;
}

std::pair<ecs::EntityId, ecs::EntityId> OaksParcelDialogOverworldFlowState::GetPokedexSpriteEntityIds() const
{
	std::pair<ecs::EntityId, ecs::EntityId> pokedexEntityIds;

	const auto firstPokedexPosition  = TileCoordsToPosition(FIRST_POKEDEX_COORDS.mCol, FIRST_POKEDEX_COORDS.mRow);
	const auto secondPokedexPosition = TileCoordsToPosition(SECOND_POKEDEX_COORDS.mCol, SECOND_POKEDEX_COORDS.mRow);

	const auto& activeEntities = mWorld.GetActiveEntities();
	for (const auto& entityId : activeEntities)
	{
		if (mWorld.HasComponent<TransformComponent>(entityId))
		{
			const auto& transformComponent = mWorld.GetComponent<TransformComponent>(entityId);
			if
			(
				math::Abs(firstPokedexPosition.x - transformComponent.mPosition.x) < 0.01f &&
				math::Abs(firstPokedexPosition.y - transformComponent.mPosition.y) < 0.01f &&
				math::Abs(firstPokedexPosition.z - transformComponent.mPosition.z) < 0.01f
			)
			{
				pokedexEntityIds.first = entityId;
			}
			else if 
			(
				math::Abs(secondPokedexPosition.x - transformComponent.mPosition.x) < 0.01f &&
				math::Abs(secondPokedexPosition.y - transformComponent.mPosition.y) < 0.01f &&
				math::Abs(secondPokedexPosition.z - transformComponent.mPosition.z) < 0.01f
			)
			{
				pokedexEntityIds.second = entityId;
			}
		}
	}

	return pokedexEntityIds;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
