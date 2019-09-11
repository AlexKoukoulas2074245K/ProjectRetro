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
#include "common/components/ItemStatsSingletonComponent.h"
#include "common/components/MoveStatsSingletonComponent.h"
#include "common/components/TextboxComponent.h"
#include "common/components/TransformComponent.h"
#include "common/components/PlayerStateSingletonComponent.h"
#include "common/components/PlayerTagComponent.h"
#include "common/components/PokedexStateSingletonComponent.h"
#include "common/components/PokemonBaseStatsSingletonComponent.h"
#include "common/components/PokemonStatsDisplayViewStateSingletonComponent.h"
#include "common/components/PokemonSelectionViewStateSingletonComponent.h"
#include "common/components/TrainersInfoStatsSingletonComponent.h"
#include "common/systems/GuiManagementSystem.h"
#include "common/utils/PokemonItemsUtils.h"
#include "common/utils/PokemonMoveUtils.h"
#include "common/utils/PokemonUtils.h"
#include "common/utils/TrainerUtils.h"
#include "encounter/components/EncounterStateSingletonComponent.h"
#include "encounter/systems/EncounterShakeControllerSystem.h"
#include "encounter/systems/EncounterStateControllerSystem.h"
#include "input/components/InputStateSingletonComponent.h"
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
#include "overworld/systems/AnimatedFlowersAnimationSystem.h"
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

#include <SDL_events.h> 
#include <SDL_timer.h>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float App::MIN_DT = 0.00001f;
const float App::MAX_DT = 1.0f;

#ifndef NDEBUG
const float App::DEBUG_DT_SPEEDUP = 10.0f;
#endif

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void App::Run()
{    
    CreateSystems();
    GameLoop();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void App::CreateSystems()
{
    auto renderingSystem = std::make_unique<RenderingSystem>(mWorld);

    mWorld.AddSystem(std::make_unique<RawInputHandlingSystem>(mWorld));
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
    mWorld.AddSystem(std::move(renderingSystem));
}

void App::GameLoop()
{    
    float elapsedTicks          = 0.0f;
    float dtAccumulator         = 0.0f;
    long long framesAccumulator = 0;
    
    DummyInitialization();

    const auto& windowComponent              = mWorld.GetSingletonComponent<WindowSingletonComponent>();
    const auto& renderingContextComponent    = mWorld.GetSingletonComponent<RenderingContextSingletonComponent>();
     
#ifndef NDEBUG
    const auto& inputStateSingletonComponent = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
#endif
    
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
            const auto fpsString            = " - FPS: " + std::to_string(framesAccumulator);
            const auto frustumCulledString  = " - FCed: " + std::to_string(renderingContextComponent.mFrustumCulledEntities);
            const auto entityCountString    = " - Entities: " + std::to_string(mWorld.GetActiveEntities().size());
            const auto renderingCallsString = " - Render Calls: " + std::to_string(renderingContextComponent.mRenderedEntities);
            SDL_SetWindowTitle(windowComponent.mWindowHandle, (windowComponent.mWindowTitle + fpsString + frustumCulledString + entityCountString + renderingCallsString).c_str());

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

void App::DummyInitialization()
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

    auto pokedexStateComponent = std::make_unique<PokedexStateSingletonComponent>();
    pokedexStateComponent->mPokedexUnlocked = true;    
    mWorld.SetSingletonComponent<PokedexStateSingletonComponent>(std::move(pokedexStateComponent));

    mWorld.SetSingletonComponent<PokemonSelectionViewStateSingletonComponent>(std::make_unique<PokemonSelectionViewStateSingletonComponent>());
    mWorld.SetSingletonComponent<PokemonStatsDisplayViewStateSingletonComponent>(std::make_unique<PokemonStatsDisplayViewStateSingletonComponent>());
    mWorld.SetSingletonComponent<EvolutionAnimationStateSingletonComponent>(std::make_unique<EvolutionAnimationStateSingletonComponent>());

    auto playerStateComponent = std::make_unique<PlayerStateSingletonComponent>();    
    playerStateComponent->mTrainerId = math::RandomInt(0, 65535);
    playerStateComponent->mPokeDollarCredits = 3000;
    playerStateComponent->mPlayerTrainerName = StringId("Alex");
    playerStateComponent->mRivalName  = StringId("Gary");
    playerStateComponent->mPlayerPokemonRoster.push_back(CreatePokemon(StringId("PIKACHU"), 6, false, mWorld));    
    
    playerStateComponent->mPlayerPokemonRoster.front()->mXpPoints += 50;
    for (const auto& pokemonEntry : playerStateComponent->mPlayerPokemonRoster)
    {
        ChangePokedexEntryForPokemon(pokemonEntry->mBaseSpeciesStats.mSpeciesName, PokedexEntryType::OWNED, mWorld);
    }

    mWorld.SetSingletonComponent<PlayerStateSingletonComponent>(std::move(playerStateComponent));

    InitializePlayerBag(mWorld);

    AddItemToBag(StringId("POTION"), mWorld, 2);
    AddItemToBag(StringId("POK^_BALL"), mWorld, 2);
    AddItemToBag(StringId("GREAT_BALL"), mWorld, 2);    
    AddItemToBag(StringId("GREAT_BALL"), mWorld, 2);
    AddItemToBag(StringId("NUGGET"), mWorld, 2);
    AddItemToBag(StringId("FOSSIL"), mWorld, 2);
    AddItemToBag(StringId("OLD_AMBER"), mWorld, 2);
    AddItemToBag(StringId("REPEL"), mWorld, 2);

    const auto levelEntityId  = LoadAndCreateLevelByName(StringId("in_viridian_poke_center"), mWorld);
    auto& levelModelComponent = mWorld.GetComponent<LevelModelComponent>(levelEntityId);

    auto activeLevelComponent = std::make_unique<ActiveLevelSingletonComponent>();
    activeLevelComponent->mActiveLevelNameId = levelModelComponent.mLevelName;
    mWorld.SetSingletonComponent<ActiveLevelSingletonComponent>(std::move(activeLevelComponent));

    CreatePlayerOverworldSprite(levelEntityId, Direction::SOUTH, 12, 5, mWorld);

    SoundService::GetInstance().PlayMusic(levelModelComponent.mLevelMusicTrackName);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
