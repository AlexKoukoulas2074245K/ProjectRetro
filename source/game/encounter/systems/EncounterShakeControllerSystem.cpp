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
#include "../../common/utils/TextboxUtils.h"
#include "../../rendering/components/CameraSingletonComponent.h"
#include "../../common/components/TransformComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 EncounterShakeControllerSystem::OPPONENT_STATUS_DISPLAY_POSITION       = glm::vec3(-0.32f, 0.7f, 0.4f);
const glm::vec3 EncounterShakeControllerSystem::OPPONENT_STATUS_DISPLAY_SCALE          = glm::vec3(1.0f, 1.0f, 1.0f);
const glm::vec3 EncounterShakeControllerSystem::OPPONENT_SPRITE_POSITION               = glm::vec3(0.38f, 0.61f, 0.3f);
const glm::vec3 EncounterShakeControllerSystem::OPPONENT_SPRITE_SCALE                  = glm::vec3(0.49f, 0.49f, 1.0f);
const glm::vec3 EncounterShakeControllerSystem::OPPONENT_POKEMON_INFO_TEXTBOX_POSITION = glm::vec3(0.04f, 0.858f, 0.35f);

const int EncounterShakeControllerSystem::OPPONENT_POKEMON_INFO_TEXTBOX_COLS = 20;
const int EncounterShakeControllerSystem::OPPONENT_POKEMON_INFO_TEXTBOX_ROWS = 2;

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
    
    if (shakeComponent.mActiveShakeType == ShakeType::PLAYER_POKEMON_STATUS_SHAKE)
    {
        UpdatePlayerPokemonStatusShake(dt);
    }
    else if(shakeComponent.mActiveShakeType == ShakeType::OPPONENT_POKEMON_LONG_HORIZONTAL_SHAKE)
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
                case ShakeType::OPPONENT_POKEMON_STATUS_SHAKE:           UpdateOpponentPokemonStatusShake(dt); break;
                case ShakeType::PLAYER_POKEMON_VERTICAL_SHAKE:           UpdatePlayerPokemonVerticalShake(dt); break;
                case ShakeType::PLAYER_RAPID_LONG_HORIZONTAL_SHAKE:      UpdatePlayerPokemonRapidLongHorizontalShake(dt); break;

                // These two are updated based on interpolation rather than fixed offsets. See above
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
            mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);
            encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId = ecs::NULL_ENTITY_ID;
        }
        else
        {            
            encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId = LoadAndCreatePokemonSprite
            (
                encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex]->mBaseSpeciesStats.mSpeciesName,
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

void EncounterShakeControllerSystem::UpdateOpponentPokemonStatusShake(const float) const
{
    const auto& encounterStateComponent    = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    auto& opponentSpriteTransformComponent = mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mOpponentActiveSpriteEntityId);    
    auto& shakeComponent                   = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();    

    if (shakeComponent.mShakeProgressionStep == 17)
    {
        opponentSpriteTransformComponent.mPosition.x = OPPONENT_SPRITE_POSITION.x;
        RedrawOpponentStatusDisplay(0.0f);
        shakeComponent.mActiveShakeType = ShakeType::NONE;
    }
    else if (shakeComponent.mShakeProgressionStep % 2 == 0)
    {        
        opponentSpriteTransformComponent.mPosition.x = OPPONENT_SPRITE_POSITION.x - 2 * GUI_PIXEL_SIZE;
        RedrawOpponentStatusDisplay(-2 * GUI_PIXEL_SIZE);
    }
    else 
    {
        opponentSpriteTransformComponent.mPosition.x = OPPONENT_SPRITE_POSITION.x + 2 * GUI_PIXEL_SIZE;
        RedrawOpponentStatusDisplay( 2 * GUI_PIXEL_SIZE);
    }

    shakeComponent.mShakeProgressionStep++;
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

void EncounterShakeControllerSystem::UpdatePlayerPokemonStatusShake(const float dt) const
{
    auto& shakeComponent  = mWorld.GetSingletonComponent<EncounterShakeSingletonComponent>();
    auto& cameraComponent = mWorld.GetSingletonComponent<CameraSingletonComponent>();

    // Rapid long horizontal shake
    if (shakeComponent.mShakeProgressionStep <= 15)
    {
        shakeComponent.mShakeTimeDelayTimer->Update(dt);
        if (shakeComponent.mShakeTimeDelayTimer->HasTicked())
        {
            shakeComponent.mShakeTimeDelayTimer->Reset();

            if (shakeComponent.mShakeProgressionStep == 15)
            {
                cameraComponent.mGlobalScreenOffset.x = 0.0f;
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
    }
    else
    {
        switch (shakeComponent.mShakeProgressionStep)
        {
            case 16:
            case 18:
            {
                cameraComponent.mGlobalScreenOffset.x += 0.3f * dt;
                if (cameraComponent.mGlobalScreenOffset.x >= 6.0f * GUI_PIXEL_SIZE)
                {
                    cameraComponent.mGlobalScreenOffset.x = 6.0f * GUI_PIXEL_SIZE;
                    shakeComponent.mShakeProgressionStep++;
                }
            } break;

            case 17:
            case 19:
            {
                cameraComponent.mGlobalScreenOffset.x -= 0.3f * dt;
                if (cameraComponent.mGlobalScreenOffset.x <= 0.0f)
                {
                    cameraComponent.mGlobalScreenOffset.x = 0.0f;
                    shakeComponent.mShakeProgressionStep++;
                }
            } break;

            case 20:
            {
                cameraComponent.mGlobalScreenOffset.y = 0.0f;
                shakeComponent.mActiveShakeType = ShakeType::NONE;
            }
        }
    }
}

void EncounterShakeControllerSystem::RedrawOpponentStatusDisplay(const float dx) const
{
    auto& encounterStateComponent     = mWorld.GetSingletonComponent<EncounterStateSingletonComponent>();
    const auto& activeOpponentPokemon = *encounterStateComponent.mOpponentPokemonRoster[encounterStateComponent.mActiveOpponentPokemonRosterIndex];

    mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId);
    mWorld.DestroyEntity(encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId);
    DestroyGenericOrBareTextbox(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, mWorld);

    encounterStateComponent.mViewObjects.mOpponentStatusDisplayEntityId = LoadAndCreateOpponentPokemonStatusDisplay
    (
        OPPONENT_STATUS_DISPLAY_POSITION + glm::vec3(dx, 0.0f, 0.0f),
        OPPONENT_STATUS_DISPLAY_SCALE,
        mWorld
    );

    //TODO: select appropriate bar color
    encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId = LoadAndCreatePokemonHealthBar
    (
        static_cast<float>(activeOpponentPokemon.mHp) / activeOpponentPokemon.mMaxHp,
        true,
        mWorld
    );

    mWorld.GetComponent<TransformComponent>(encounterStateComponent.mViewObjects.mOpponentPokemonHealthBarEntityId).mPosition.x += dx;

    // Create opponent pokemon name and level textbox
    encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId = CreateTextboxWithDimensions
    (
        TextboxType::BARE_TEXTBOX,
        OPPONENT_POKEMON_INFO_TEXTBOX_COLS,
        OPPONENT_POKEMON_INFO_TEXTBOX_ROWS,
        OPPONENT_POKEMON_INFO_TEXTBOX_POSITION.x + dx,
        OPPONENT_POKEMON_INFO_TEXTBOX_POSITION.y,
        OPPONENT_POKEMON_INFO_TEXTBOX_POSITION.z,
        mWorld
    );

    // Write opponent pokemon name string
    const auto opponentPokemonName = activeOpponentPokemon.mName.GetString();
    WriteTextAtTextboxCoords(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, opponentPokemonName, 0, 0, mWorld);

    // Write opponent pokemon level string
    const auto opponentPokemonLevel = activeOpponentPokemon.mLevel;
    WriteTextAtTextboxCoords(encounterStateComponent.mViewObjects.mOpponentPokemonInfoTextboxEntityId, "=" + std::to_string(opponentPokemonLevel), 3, 1, mWorld);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
