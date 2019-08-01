//
//  EncounterShakeControllerSystem.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef EncounterShakeControllerSystem_h
#define EncounterShakeControllerSystem_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class EncounterShakeControllerSystem final: public ecs::BaseSystem
{
public:
    EncounterShakeControllerSystem(ecs::World&);
    
    void VUpdateAssociatedComponents(const float dt) const override;
    
private:
    void InitializeShakeComponent() const;
    void UpdateOpponentPokemonBlink(const float dt) const;
    void UpdateOpponentPokemonShortHorizontalShake(const float dt) const;
    void UpdateOpponentPokemonLongHorizontalShake(const float dt) const;
    void UpdateOpponentPokemonStatusShake(const float dt) const;
    void UpdatePlayerPokemonVerticalShake(const float dt) const;
    void UpdatePlayerPokemonLongHorizontalShake(const float dt) const;
    void UpdatePlayerPokemonRapidLongHorizontalShake(const float dt) const;
    void UpdatePlayerPokemonStatusShake(const float dt) const;
    void RedrawOpponentStatusDisplay(const float dx) const;

    static const glm::vec3 OPPONENT_STATUS_DISPLAY_POSITION;
    static const glm::vec3 OPPONENT_STATUS_DISPLAY_SCALE;
    static const glm::vec3 OPPONENT_SPRITE_POSITION;
    static const glm::vec3 OPPONENT_SPRITE_SCALE;
    static const glm::vec3 OPPONENT_POKEMON_INFO_TEXTBOX_POSITION;

    static const int OPPONENT_POKEMON_INFO_TEXTBOX_COLS;
    static const int OPPONENT_POKEMON_INFO_TEXTBOX_ROWS;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* EncounterShakeControllerSystem_h */
