//
//  PokemonStatsDisplayViewFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 23/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokemonStatsDisplayViewFlowState_h
#define PokemonStatsDisplayViewFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokemonStatsDisplayViewFlowState final: public BaseFlowState
{
public:
    PokemonStatsDisplayViewFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    void CreatePokemonStatsBackground() const;
    void LoadAndCreatePokemonStatsScreen1() const;
    void LoadAndCreatePokemonStatsScreen2() const;
    void DestroyPokemonStatsScreen() const;    
    void DestroyPokemonStatsBackground() const;

    static const glm::vec3 BACKGROUND_POSITION;
    static const glm::vec3 BACKGROUND_SCALE;
    static const glm::vec3 POKEMON_SPRITE_POSITION;
    static const glm::vec3 POKEMON_SPRITE_SCALE;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonStatsDisplayViewFlowState_h */