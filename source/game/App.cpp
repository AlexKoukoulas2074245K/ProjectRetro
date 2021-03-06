//
//  App.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "App.h"
#include "common/components/DirectionComponent.h"
#include "common/components/EvolutionAnimationStateSingletonComponent.h"
#include "common/components/GameIntroStateSingletonComponent.h"
#include "common/components/ItemStatsSingletonComponent.h"
#include "common/components/MarketStocksSingletonComponent.h"
#include "common/components/MoveStatsSingletonComponent.h"
#include "common/components/NameSelectionStateSingletonComponent.h"
#include "common/components/PokeMartDialogStateSingletonComponent.h"
#include "common/components/TextboxComponent.h"
#include "common/components/TransformComponent.h"
#include "common/components/PlayerStateSingletonComponent.h"
#include "common/components/PlayerTagComponent.h"
#include "common/components/PokedexStateSingletonComponent.h"
#include "common/components/PokemonBaseStatsSingletonComponent.h"
#include "common/components/PokemonStatsDisplayViewStateSingletonComponent.h"
#include "common/components/PokemonSelectionViewStateSingletonComponent.h"
#include "common/components/TrainersInfoStatsSingletonComponent.h"
#include "common/flowstates/GameIntroFlowState.h"
#include "common/systems/GuiManagementSystem.h"
#include "common/utils/PersistenceUtils.h"
#include "common/utils/PokedexUtils.h"
#include "common/utils/PokemonItemsUtils.h"
#include "common/utils/PokemonMoveUtils.h"
#include "common/utils/PokemonUtils.h"
#include "common/utils/TrainerUtils.h"
#include "encounter/components/EncounterStateSingletonComponent.h"
#include "encounter/systems/EncounterShakeControllerSystem.h"
#include "encounter/systems/EncounterStateControllerSystem.h"
#include "input/components/InputStateSingletonComponent.h"
#include "input/systems/AiInputControlSystem.h"
#include "input/systems/RawInputHandlingSystem.h"
#include "input/utils/InputUtils.h"
#include "rendering/components/AnimationTimerComponent.h"
#include "rendering/components/RenderableComponent.h"
#include "rendering/components/RenderingContextSingletonComponent.h"
#include "rendering/components/WindowSingletonComponent.h"
#include "rendering/systems/AnimationSystem.h"
#include "rendering/systems/CameraControlSystem.h"
#include "rendering/systems/RenderingSystem.h"
#include "resources/MeshUtils.h"
#include "resources/ResourceLoadingService.h"
#include "resources/SfxResource.h"
#include "resources/MusicResource.h"
#include "sound/SoundService.h"
#include "overworld/components/ActiveLevelSingletonComponent.h"
#include "overworld/components/LevelResidentComponent.h"
#include "overworld/components/LevelModelComponent.h"
#include "overworld/components/MovementStateComponent.h"
#include "overworld/components/PCStateSingletonComponent.h"
#include "overworld/components/PokeCenterHealingAnimationStateSingletonComponent.h"
#include "overworld/components/TownMapLocationDataSingletonComponent.h"
#include "overworld/systems/AnimatedFlowersAnimationSystem.h"
#include "overworld/systems/MilestoneAlterationsSystem.h"
#include "overworld/systems/MovementControllerSystem.h"
#include "overworld/systems/NpcAiSystem.h"
#include "overworld/systems/OverworldFlowControllerSystem.h"
#include "overworld/systems/PlayerActionControllerSystem.h"
#include "overworld/systems/SeaTilesAnimationSystem.h"
#include "overworld/systems/TransitionAnimationSystem.h"
#include "overworld/systems/WarpConnectionsSystem.h"
#include "overworld/utils/LevelUtils.h"
#include "overworld/utils/LevelLoadingUtils.h"
#include "overworld/utils/OverworldCharacterLoadingUtils.h"
#include "overworld/utils/OverworldUtils.h"
#include "overworld/utils/TownMapUtils.h"

#include <SDL_events.h> 
#include <SDL_timer.h>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float App::MIN_DT = 0.00001f;
const float App::MAX_DT = 1.0f;
const float App::DEBUG_DT_SPEEDUP = 10.0f;

const std::string App::WINDOW_TITLE = "Pokemon Yellow 3D";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void App::Run()
{    
    CreateSystems();
    SingletonComponentsInitialization();
    GameLoop();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void App::CreateSystems()
{
    auto renderingSystem = std::make_unique<RenderingSystem>(mWorld);

    mWorld.AddSystem(std::make_unique<RawInputHandlingSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<AiInputControlSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<GuiManagementSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<PlayerActionControllerSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<NpcAiSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<AnimationSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<MovementControllerSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<AnimatedFlowersAnimationSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<SeaTilesAnimationSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<TransitionAnimationSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<WarpConnectionsSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<EncounterShakeControllerSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<EncounterStateControllerSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<OverworldFlowControllerSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<CameraControlSystem>(mWorld));
    mWorld.AddSystem(std::make_unique<MilestoneAlterationsSystem>(mWorld));
    mWorld.AddSystem(std::move(renderingSystem));
}

void App::GameLoop()
{    
    float elapsedTicks          = 0.0f;
    float dtAccumulator         = 0.0f;
    long long framesAccumulator = 0;        
    
    if (DoesSaveFileExist())
    {
        InitializationFromSaveFile();
    }
    else
    {
        StartIntroSequence();
    }
    
    const auto& windowComponent              = mWorld.GetSingletonComponent<WindowSingletonComponent>();
    const auto& renderingContextComponent    = mWorld.GetSingletonComponent<RenderingContextSingletonComponent>();
    const auto& inputStateSingletonComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();

    SDL_SetWindowTitle(windowComponent.mWindowHandle, WINDOW_TITLE.c_str());

    auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();

    mWorld.OnPreFirstUpdate();
    
    while (!AppShouldQuit())
    {
        // Calculate frame delta
        const auto currentTicks = static_cast<float>(SDL_GetTicks());
        auto lastFrameTicks     = currentTicks - elapsedTicks;
        elapsedTicks            = currentTicks;
        const auto dt           = lastFrameTicks * 0.001f;

        framesAccumulator++;
        dtAccumulator += dt;
        
        if (dtAccumulator > 1.0f)
        {
            playerStateComponent.mSecondsPlayed++;

            const auto fpsString            = " - FPS: " + std::to_string(framesAccumulator);
            const auto frustumCulledString  = " - FCed: " + std::to_string(renderingContextComponent.mFrustumCulledEntities);
            const auto entityCountString    = " - Entities: " + std::to_string(mWorld.GetActiveEntities().size());
            const auto renderingCallsString = " - Render Calls: " + std::to_string(renderingContextComponent.mRenderedEntities);
            SDL_SetWindowTitle(windowComponent.mWindowHandle, (windowComponent.mWindowTitle + fpsString + frustumCulledString + entityCountString + renderingCallsString).c_str());
           
#ifdef NDEBUG
            SDL_SetWindowTitle(windowComponent.mWindowHandle, (WINDOW_TITLE + fpsString).c_str());
#endif
            framesAccumulator = 0;
            dtAccumulator = 0.0f;
        }                
        
#ifndef NDEBUG          
        if (IsActionTypeKeyPressed(VirtualActionType::DEBUG_SPEED_UP, inputStateSingletonComponent))
        {
            mWorld.Update(math::Max(MIN_DT, math::Min(dt * DEBUG_DT_SPEEDUP, MAX_DT)));
        }
        else
        {
            mWorld.Update(math::Max(MIN_DT, math::Min(dt, MAX_DT)));
        }
#else    
        mWorld.Update(math::Max(MIN_DT, math::Min(dt, MAX_DT)));
#endif      
        

        if (IsActionTypeKeyTapped(VirtualActionType::DEBUG_CAMERA_BACKWARD, inputStateSingletonComponent))
        {
            SoundService::GetInstance().ToggleAudioOnOff();
        }
    }
}

bool App::AppShouldQuit()
{
    // Poll events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT: return true;
        }
    }

    return false;
}

void App::SingletonComponentsInitialization()
{
    auto pokemonBaseStatsComponent = std::make_unique<PokemonBaseStatsSingletonComponent>();
    LoadAndPopulatePokemonBaseStats(*pokemonBaseStatsComponent);
    mWorld.SetSingletonComponent<PokemonBaseStatsSingletonComponent>(std::move(pokemonBaseStatsComponent));
    
    auto trainerInfoStatsComponent = std::make_unique<TrainersInfoStatsSingletonComponent>();
    LoadAndPopulateTrainerInfoStats(*trainerInfoStatsComponent);
    mWorld.SetSingletonComponent<TrainersInfoStatsSingletonComponent>(std::move(trainerInfoStatsComponent));
    
    auto moveStatsComponent = std::make_unique<MoveStatsSingletonComponent>();
    LoadAndPopulateMoveStats(*moveStatsComponent);
    mWorld.SetSingletonComponent<MoveStatsSingletonComponent>(std::move(moveStatsComponent));
    
    auto itemStatsComponent = std::make_unique<ItemStatsSingletonComponent>();
    LoadAndPopulateItemsStats(*itemStatsComponent);
    mWorld.SetSingletonComponent<ItemStatsSingletonComponent>(std::move(itemStatsComponent));    
    
    auto townMapDataComponent = std::make_unique<TownMapLocationDataSingletonComponent>();
    LoadAndPopulateTownMapLocationData(*townMapDataComponent);
    mWorld.SetSingletonComponent<TownMapLocationDataSingletonComponent>(std::move(townMapDataComponent));

    auto marketStocksComponent = std::make_unique<MarketStocksSingletonComponent>();
    LoadAndPopulateMarketStocks(*marketStocksComponent);
    mWorld.SetSingletonComponent<MarketStocksSingletonComponent>(std::move(marketStocksComponent));
    
    mWorld.SetSingletonComponent<GameIntroStateSingletonComponent>(std::make_unique<GameIntroStateSingletonComponent>());
    mWorld.SetSingletonComponent<NameSelectionStateSingletonComponent>(std::make_unique<NameSelectionStateSingletonComponent>());
    mWorld.SetSingletonComponent<PokedexStateSingletonComponent>(std::make_unique<PokedexStateSingletonComponent>());
    mWorld.SetSingletonComponent<PokemonSelectionViewStateSingletonComponent>(std::make_unique<PokemonSelectionViewStateSingletonComponent>());
    mWorld.SetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>(std::make_unique<PokemonStatsDisplayViewStateSingletonComponent>());
    mWorld.SetSingletonComponent<EvolutionAnimationStateSingletonComponent>(std::make_unique<EvolutionAnimationStateSingletonComponent>());
    mWorld.SetSingletonComponent<PokeCenterHealingAnimationStateSingletonComponent>(std::make_unique<PokeCenterHealingAnimationStateSingletonComponent>());
    mWorld.SetSingletonComponent<PokeMartDialogStateSingletonComponent>(std::make_unique<PokeMartDialogStateSingletonComponent>());
    mWorld.SetSingletonComponent<PCStateSingletonComponent>(std::make_unique<PCStateSingletonComponent>());
}

void App::InitializationFromSaveFile()
{
    RestoreGameStateFromSaveFile(mWorld);
}

void App::StartIntroSequence()
{
    auto playerStateComponent = std::make_unique<PlayerStateSingletonComponent>();
    playerStateComponent->mSecondsPlayed     = 0;
    playerStateComponent->mTrainerId         = math::RandomInt(0, MAX_TRAINER_ID);
    playerStateComponent->mPokeDollarCredits = PLAYER_STARTING_POKE_DOLLARS;

    mWorld.SetSingletonComponent<PlayerStateSingletonComponent>(std::move(playerStateComponent));

    InitializePlayerBag(mWorld);

    const auto levelEntityId = LoadAndCreateLevelByName(StringId("intro"), mWorld);
    auto& levelModelComponent = mWorld.GetComponent<LevelModelComponent>(levelEntityId);

    auto activeLevelComponent = std::make_unique<ActiveLevelSingletonComponent>();
    activeLevelComponent->mActiveLevelNameId = levelModelComponent.mLevelName;
    mWorld.SetSingletonComponent<ActiveLevelSingletonComponent>(std::move(activeLevelComponent));    

    SoundService::GetInstance().PlayMusic(levelModelComponent.mLevelMusicTrackName);

    StartOverworldFlowState<GameIntroFlowState>(mWorld);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
