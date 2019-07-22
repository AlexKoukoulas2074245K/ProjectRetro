//
//  PokemonSelectionViewFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 19/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokemonSelectionViewFlowState_h
#define PokemonSelectionViewFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../utils/MathUtils.h"
#include "../GameConstants.h"
#include "../../ECS.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokemonSelectionViewFlowState final: public BaseFlowState
{
public:
    PokemonSelectionViewFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    void PokemonSelectedFlow();
    void PokemonNotSelectedFlow();
    void DisplayPokemonDetailedStatsFlow();
    void SwitchPokemonFlow();
    void PokemonRosterIndexSwapFlow();
    void CancelPokemonSelectionFlow();

    void CreatePokemonSelectionBackground() const;
    void CreateIndividualPokemonSprites() const;
    void CreatePokemonStatsInvisibleTextbox() const;
    void DestroyPokemonSelectionView() const;
    
    std::string GetCursorCommandTextFirstFourLetters() const;
    ecs::EntityId CreatePokemonOverworldSprite
    (
        const OverworldPokemonSpriteType,
        const float healthRemainingProportion, 
        const int row
    ) const;
    
    static const std::string POKEMON_SELECTION_VIEW_BACKGROUND_MODEL_FILE_NAME;
    static const std::string POKEMON_SELECTION_VIEW_BACKGROUND_TEXTURE_FILE_NAME;
    static const std::string POKEMON_SPRITE_MODEL_NAME;
    static const std::string POKEMON_SPRITE_ATLAS_TEXTURE_FILE_NAME;

    static const glm::vec3 POKEMON_SELECTION_VIEW_BACKGROUND_POSITION;
    static const glm::vec3 POKEMON_SELECTION_VIEW_BACKGROUND_SCALE;
    static const glm::vec3 POKEMON_SELECTION_VIEW_STATS_TEXTBOX_BASE_POSITION;
    static const glm::vec3 POKEMON_SELECTION_OVERWORLD_SPRITE_BASE_POSITION;

    static const float POKEMON_SELECTION_VIEW_SPRITE_ANIMATION_FRAME_DURATION_SLOW;
    static const float POKEMON_SELECTION_VIEW_SPRITE_ANIMATION_FRAME_DURATION_MEDIUM;
    static const float POKEMON_SELECTION_VIEW_SPRITE_ANIMATION_FRAME_DURATION_FAST;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonSelectionViewFlowState_h */
