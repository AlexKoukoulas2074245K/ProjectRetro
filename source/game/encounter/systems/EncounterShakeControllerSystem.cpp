//
//  EncounterShakeControllerSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "EncounterShakeControllerSystem.h"
#include "../components/EncounterShakeSingletonComponent.h"
#include "../../common/utils/OSMessageBox.h"
#include "../../rendering/components/CameraSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

EncounterShakeControllerSystem::EncounterShakeControllerSystem(ecs::World& world)
    : BaseSystem(world)
{
    InitializeShakeComponent();
}

void EncounterShakeControllerSystem::VUpdateAssociatedComponents(const float dt) const
{
    auto& shakeComponent = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    
    switch (shakeComponent.mActiveShakeType)
    {
        case ShakeType::NONE:                                    break;
        case ShakeType::OPPONENT_POKEMON_BLINK:                  UpdateOpponentPokemonBlink(dt); break;
        case ShakeType::OPPONENT_POKEMON_SHORT_HORIZONTAL_SHAKE: UpdateOpponentPokemonShortHorizontalShake(dt); break;
        case ShakeType::OPPONENT_POKEMON_LONG_HORIZONTAL_SHAKE:  UpdateOpponentPokemonLongHorizontalShake(dt); break;
        case ShakeType::PLAYER_POKEMON_VERTICAL_SHAKE:           UpdatePlayerPokemonVerticalShake(dt); break;
        case ShakeType::PLAYER_POKEMON_LONG_HORIZONTAL_SHAKE:    UpdatePlayerPokmeonLongHorizontalShake(dt); break;
        case ShakeType::PLAYER_RAPID_LONG_HORIZONTAL_SHAKE:      UpdatePlayerPokemonRapidLongHorizontalShake(dt); break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void EncounterShakeControllerSystem::InitializeShakeComponent() const
{
    auto encounterShakeComponent                  = std::make_unique<EncounterShakeSingletonComponent>();
    encounterShakeComponent->mShakeTimeDelayTimer = std::make_unique<Timer>(SHAKE_TIME_DELAY);
    
    mWorld.SetSingletonComponent<EncounterShakeSingletonComponent>(std::move(encounterShakeComponent));
}

void EncounterShakeControllerSystem::UpdateOpponentPokemonBlink(const float) const
{
    auto& shakeComponent  = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    shakeComponent.mActiveShakeType = ShakeType::NONE;
    ShowMessageBox(MessageBoxType::INFO, "Missing Shake Flow", "Opponent Pokemon Blink Shake not implemented");
}

void EncounterShakeControllerSystem::UpdateOpponentPokemonShortHorizontalShake(const float dt) const
{
    auto& shakeComponent  = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    auto& cameraComponent = mWorld.GetSingletonComponent<CameraSingletonComponent>();
    
    shakeComponent.mShakeTimeDelayTimer->Update(dt);
    if (shakeComponent.mShakeTimeDelayTimer->HasTicked())
    {
        shakeComponent.mShakeTimeDelayTimer->Reset();
        
        switch (shakeComponent.mShakeProgressionStep)
        {
            case 0:
            {
                cameraComponent.mGlobalScreenOffset.x = SHAKE_DISTANCE_UNIT * 2;
                shakeComponent.mShakeProgressionStep++;
            } break;
                
            case 1:
            {
                cameraComponent.mGlobalScreenOffset.x = 0.0f;
                shakeComponent.mShakeProgressionStep++;
            } break;
                
            case 2:
            {
                cameraComponent.mGlobalScreenOffset.x = SHAKE_DISTANCE_UNIT;
                shakeComponent.mShakeProgressionStep++;
            } break;
                
            case 3:
            {
                cameraComponent.mGlobalScreenOffset.x = 0.0f;
                shakeComponent.mShakeProgressionStep++;
            } break;
                
            case 4:
            {
                shakeComponent.mShakeProgressionStep = 0;
                shakeComponent.mActiveShakeType = ShakeType::NONE;
            } break;
        }
    }
}

void EncounterShakeControllerSystem::UpdateOpponentPokemonLongHorizontalShake(const float) const
{
    auto& shakeComponent = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    shakeComponent.mActiveShakeType = ShakeType::NONE;
    ShowMessageBox(MessageBoxType::INFO, "Missing Shake Flow", "Opponent Pokemon Long Horizontal Shake not implemented");
}

void EncounterShakeControllerSystem::UpdatePlayerPokemonVerticalShake(const float) const
{
    auto& shakeComponent = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    shakeComponent.mActiveShakeType = ShakeType::NONE;
    ShowMessageBox(MessageBoxType::INFO, "Missing Shake Flow", "Player Pokemon Vertical Shake not implemented");
}

void EncounterShakeControllerSystem::UpdatePlayerPokmeonLongHorizontalShake(const float) const
{
    auto& shakeComponent = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    shakeComponent.mActiveShakeType = ShakeType::NONE;
    ShowMessageBox(MessageBoxType::INFO, "Missing Shake Flow", "Player Pokemon Long Horizontal Shake not implemented");
}

void EncounterShakeControllerSystem::UpdatePlayerPokemonRapidLongHorizontalShake(const float) const
{
    auto& shakeComponent = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    shakeComponent.mActiveShakeType = ShakeType::NONE;
    ShowMessageBox(MessageBoxType::INFO, "Missing Shake Flow", "Player Pokemon Rapid Long Horizontal Shake not implemented");
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
