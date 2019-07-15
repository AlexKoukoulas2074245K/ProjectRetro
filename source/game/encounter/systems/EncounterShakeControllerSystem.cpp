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
#include "../components/EncounterStateSingletonComponent.h"
#include "../utils/EncounterSpriteUtils.h"
#include "../../common/GameConstants.h"
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
    
    shakeComponent.mShakeTimeDelayTimer->Update(dt);
    if (shakeComponent.mShakeTimeDelayTimer->HasTicked())
    {
        shakeComponent.mShakeTimeDelayTimer->Reset();

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
    auto& encounterStateComponent = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& shakeComponent          = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();

    if (shakeComponent.mShakeProgressionStep == 24)
    {
        shakeComponent.mActiveShakeType = ShakeType::NONE;
    }            
    else if (shakeComponent.mShakeProgressionStep % 2 == 0)
    {
        if (encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId != ecs::NULL_ENTITY_ID)
        {
            mWorld.RemoveEntity(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
            encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId = ecs::NULL_ENTITY_ID;
        }
        else
        {            
            encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId = LoadAndCreatePokemonSprite
            (
                encounterStateComponent.mOpponentPokemonRoster.front()->mName,
                true,
                // TODO: fix when sprite positions and other battle ves are handled differently
                glm::vec3(0.38f, 0.61f, 0.1f),
                glm::vec3(0.49f, 0.49f, 1.0f),
                mWorld
            );
        }
        
    }    

    shakeComponent.mShakeProgressionStep++;
}

void EncounterShakeControllerSystem::UpdateOpponentPokemonShortHorizontalShake(const float) const
{
    auto& shakeComponent  = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    auto& cameraComponent = mWorld.GetSingletonComponent<CameraSingletonComponent>();
        
    switch (shakeComponent.mShakeProgressionStep)
    {
        case 0:
        {
            cameraComponent.mGlobalScreenOffset.x = GUI_PIXEL_SIZE * 2;
            shakeComponent.mShakeProgressionStep++;
        } break;
                
        case 1:
        {
            cameraComponent.mGlobalScreenOffset.x = 0.0f;
            shakeComponent.mShakeProgressionStep++;
        } break;
                
        case 2:
        {
            cameraComponent.mGlobalScreenOffset.x = GUI_PIXEL_SIZE;
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

void EncounterShakeControllerSystem::UpdateOpponentPokemonLongHorizontalShake(const float) const
{
    auto& shakeComponent = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    shakeComponent.mActiveShakeType = ShakeType::NONE;
    ShowMessageBox(MessageBoxType::INFO, "Missing Shake Flow", "Opponent Pokemon Long Horizontal Shake not implemented");
}

void EncounterShakeControllerSystem::UpdatePlayerPokemonVerticalShake(const float) const
{
    auto& shakeComponent  = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    auto& cameraComponent = mWorld.GetSingletonComponent<CameraSingletonComponent>();

    if (shakeComponent.mShakeProgressionStep == 15)
    {
        cameraComponent.mGlobalScreenOffset.y = 0.0f;        
        shakeComponent.mActiveShakeType       = ShakeType::NONE;
    }
    else if (shakeComponent.mShakeProgressionStep % 2 == 0)
    {
        cameraComponent.mGlobalScreenOffset.y = 0.0f;
    }
    else
    {
        cameraComponent.mGlobalScreenOffset.y = (-8 + shakeComponent.mShakeProgressionStep / 2) * GUI_PIXEL_SIZE;
    }   

    shakeComponent.mShakeProgressionStep++;
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
