//
//  NameSelectionFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef NameSelectionFlowState_h
#define NameSelectionFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseOverworldFlowState.h"
#include "../GameConstants.h"
#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"
#include "../../ECS.h"

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class NameSelectionFlowState final: public BaseOverworldFlowState
{
public:
    NameSelectionFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    static const glm::vec3 BACKGROUND_POSITION;
    static const glm::vec3 BACKGROUND_SCALE;
    static const glm::vec3 POKEMON_SPRITE_POSITION;
    
    static const TileCoords SELECTED_NAME_START_COORDS;
    static const TileCoords SELECTED_POKEMON_NAME_END_COORDS;
    static const TileCoords SELECTED_TRAINER_NAME_END_COORDS;
    static const TileCoords END_BUTTON_COORDS;
    
    static const std::string POKEMON_SPRITE_ATLAS_TEXTURE_FILE_NAME;
    static const std::string POKEMON_SPRITE_MODEL_NAME;
    
    static const int CHARACTER_ATLAS_COLS;
    static const int CHARACTER_ATLAS_ROWS;
    
    static const float SPRITE_ANIMATION_FRAME_DURATION_MEDIUM;
    
    void CreateBackground();
    void CreatePokemonSprite(const StringId pokemonName);
    void CreateTextboxes();
    void RedrawSelectedText();
    void RepositionCursorToEnd();
    void FinishNamingFlow();
    void DestroyBackground();

    ecs::EntityId mBackgroundCoverEntityId;
    ecs::EntityId mCharactersEnclosingTextboxEntityId;
    ecs::EntityId mTitleTextboxEntityId;
    ecs::EntityId mPokemonSpriteEntityId;
    bool mUppercaseMode;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* NameSelectionFlowState_h */
