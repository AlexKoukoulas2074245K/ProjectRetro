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
#include "../../common/utils/PokemonUtils.h"
#include "../../rendering/components/CameraSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 EncounterShakeControllerSystem::OPPONENT_SPRITE_POSITION = glm::vec3(0.38f, 0.61f, -0.1f);
const glm::vec3 EncounterShakeControllerSystem::OPPONENT_SPRITE_SCALE    = glm::vec3(0.49f, 0.49f, 1.0f);

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
    
    if(shakeComponent.mActiveShakeType == ShakeType::OPPONENT_POKEMON_LONG_HORIZONTAL_SHAKE)
    {
        UpdateOpponentPokemonLongHorizontalShake(dt);
    }
    else if (shakeComponent.mActiveShakeType == ShakeType::PLAYER_POKEMON_LONG_HORIZONTAL_SHAKE)
    {
        UpdatePlayerPokemonLongHorizontalShake(dt);
    }
    else
    {
        shakeComponent.mShakeTimeDelayTimer->Update(dt);
        if (shakeComponent.mShakeTimeDelayTimer->HasTicked())
        {
            shakeComponent.mShakeTimeDelayTimer->Reset();
            
            switch (shakeComponent.mActiveShakeType)
            {
                case ShakeType::OPPONENT_POKEMON_BLINK:                  UpdateOpponentPokemonBlink(dt); break;
                case ShakeType::OPPONENT_POKEMON_SHORT_HORIZONTAL_SHAKE: UpdateOpponentPokemonShortHorizontalShake(dt); break;
                case ShakeType::PLAYER_POKEMON_VERTICAL_SHAKE:           UpdatePlayerPokemonVerticalShake(dt); break;
                case ShakeType::PLAYER_RAPID_LONG_HORIZONTAL_SHAKE:      UpdatePlayerPokemonRapidLongHorizontalShake(dt); break;
                case ShakeType::OPPONENT_POKEMON_LONG_HORIZONTAL_SHAKE:  break;
                case ShakeType::PLAYER_POKEMON_LONG_HORIZONTAL_SHAKE:    break;
                case ShakeType::NONE:                                    break;
            }
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
                GetFirstNonFaintedPokemon(encounterStateComponent.mOpponentPokemonRoster).mName,
                true,
                OPPONENT_SPRITE_POSITION,
                OPPONENT_SPRITE_SCALE,
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
        case 1:
        {
            cameraComponent.mGlobalScreenOffset.x = GUI_PIXEL_SIZE * 2;
            shakeComponent.mShakeProgressionStep++;
        } break;
                
        case 2:
        case 3:
        {
            cameraComponent.mGlobalScreenOffset.x = 0.0f;
            shakeComponent.mShakeProgressionStep++;
        } break;
                
        case 4:
        case 5:
        {
            cameraComponent.mGlobalScreenOffset.x = GUI_PIXEL_SIZE;
            shakeComponent.mShakeProgressionStep++;
        } break;
                
        case 6:
        case 7:
        {
            cameraComponent.mGlobalScreenOffset.x = 0.0f;
            shakeComponent.mShakeProgressionStep++;
        } break;
                
        case 8:
        {
            shakeComponent.mShakeProgressionStep = 0;
            shakeComponent.mActiveShakeType = ShakeType::NONE;
        } break;
    }    
}

void EncounterShakeControllerSystem::UpdateOpponentPokemonLongHorizontalShake(const float dt) const
{
    auto& shakeComponent = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    auto& cameraComponent = mWorld.GetSingletonComponent<CameraSingletonComponent>();
    
    switch (shakeComponent.mShakeProgressionStep)
    {
        case 0:
        case 2:
        {
            cameraComponent.mGlobalScreenOffset.x += 0.2f * dt;
            if (cameraComponent.mGlobalScreenOffset.x >= 3.0f * GUI_PIXEL_SIZE)
            {
                cameraComponent.mGlobalScreenOffset.x = 3.0f * GUI_PIXEL_SIZE;
                shakeComponent.mShakeProgressionStep++;
            }
        } break;
            
        case 1:
        case 3:
        {
            cameraComponent.mGlobalScreenOffset.x -= 0.2f * dt;
            if (cameraComponent.mGlobalScreenOffset.x <= 0.0f)
            {
                cameraComponent.mGlobalScreenOffset.x = 0.0f;
                shakeComponent.mShakeProgressionStep++;
            }
        } break;
            
        case 4:
        {
            cameraComponent.mGlobalScreenOffset.y = 0.0f;
            shakeComponent.mActiveShakeType = ShakeType::NONE;
        }
    }
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

void EncounterShakeControllerSystem::UpdatePlayerPokemonLongHorizontalShake(const float dt) const
{
    auto& shakeComponent = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    auto& cameraComponent = mWorld.GetSingletonComponent<CameraSingletonComponent>();

    switch (shakeComponent.mShakeProgressionStep)
    {
        case 0:
        case 2:
        {
            cameraComponent.mGlobalScreenOffset.x += 0.3f * dt;
            if (cameraComponent.mGlobalScreenOffset.x >= 6.0f * GUI_PIXEL_SIZE)
            {
                cameraComponent.mGlobalScreenOffset.x = 6.0f * GUI_PIXEL_SIZE;
                shakeComponent.mShakeProgressionStep++;
            }
        } break;

        case 1:
        case 3:
        {
            cameraComponent.mGlobalScreenOffset.x -= 0.3f * dt;
            if (cameraComponent.mGlobalScreenOffset.x <= 0.0f)
            {
                cameraComponent.mGlobalScreenOffset.x = 0.0f;
                shakeComponent.mShakeProgressionStep++;
            }
        } break;

        case 4:
        {
            cameraComponent.mGlobalScreenOffset.y = 0.0f;
            shakeComponent.mActiveShakeType = ShakeType::NONE;
        }
    }
}

void EncounterShakeControllerSystem::UpdatePlayerPokemonRapidLongHorizontalShake(const float) const
{
    auto& shakeComponent  = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    auto& cameraComponent = mWorld.GetSingletonComponent<CameraSingletonComponent>();
    
    if (shakeComponent.mShakeProgressionStep == 15)
    {
        cameraComponent.mGlobalScreenOffset.x = 0.0f;
        shakeComponent.mActiveShakeType       = ShakeType::NONE;
    }
    else if (shakeComponent.mShakeProgressionStep % 2 == 0)
    {
        cameraComponent.mGlobalScreenOffset.x = 0.0f;
    }
    else
    {
        cameraComponent.mGlobalScreenOffset.x = (-8 + shakeComponent.mShakeProgressionStep / 2) * GUI_PIXEL_SIZE;
    }
    
    shakeComponent.mShakeProgressionStep++;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
