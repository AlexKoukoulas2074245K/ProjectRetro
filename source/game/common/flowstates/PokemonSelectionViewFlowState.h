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
    void HealingUpFlow(const float dt);
    void InvalidOperationFlow();
    void PokemonSelectedFlow();
    void PokemonNotSelectedFlow();
    void PokemonSelectionViewIndexSwapFlow(const float dt);
    void DisplayPokemonDetailedStatsFlow();
    void TryUseItem();
    void TrySwitchPokemon();
    void PokemonRosterIndexSwapFlow();
    void CancelPokemonSelectionFlow();

    void CreatePokemonSelectionBackground() const;
    void CreateIndividualPokemonSprites() const;
    void CreatePokemonSelectionViewMainTextbox() const;
    void CreatePokemonStatsInvisibleTextbox() const;
    void DestroyPokemonSelectionView() const;
    
    std::string GetCursorCommandTextFirstFourLetters() const;
    ecs::EntityId CreatePokemonOverworldSprite
    (
        const OverworldPokemonSpriteType,
        const float healthRemainingProportion, 
        const int row
    ) const;
    
    void RefreshPlayerPokemonDisplayStats() const;

    static const std::string TEXTBOX_CLICK_SFX_NAME;
    static const std::string POKEMON_SWAP_SFX_NAME;
    static const std::string ITEM_HEAL_UP_SFX_NAME;    
    static const std::string POKEMON_SPRITE_MODEL_NAME;
    static const std::string POKEMON_SPRITE_ATLAS_TEXTURE_FILE_NAME;

    static const glm::vec3 BACKGROUND_POSITION;
    static const glm::vec3 BACKGROUND_SCALE;
    static const glm::vec3 STATS_TEXTBOX_BASE_POSITION;
    static const glm::vec3 OVERWORLD_SPRITE_BASE_POSITION;
    static const glm::vec3 ITEM_USAGE_RESULT_CHATBOX_POSITION;

    static const float SPRITE_ANIMATION_FRAME_DURATION_SLOW;
    static const float SPRITE_ANIMATION_FRAME_DURATION_MEDIUM;
    static const float SPRITE_ANIMATION_FRAME_DURATION_FAST;
    static const float INDEX_SWAP_ENTRY_DELAY;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonSelectionViewFlowState_h */
