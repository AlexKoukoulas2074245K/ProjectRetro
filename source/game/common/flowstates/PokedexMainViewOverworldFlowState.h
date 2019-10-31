//
//  PokedexMainViewOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 31/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokedexMainViewOverworldFlowState_h
#define PokedexMainViewOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../utils/MathUtils.h"
#include "../../ECS.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokedexMainViewOverworldFlowState final: public BaseFlowState
{
public:
    PokedexMainViewOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;

private:
    void UpdateMainView(const float dt);    
    void UpdatePokedexEntrySelected(const float dt);
    void CancelPokedexMainView();    
    void RedrawPokedexMainView() const;
    void DisplayPokedexEntriesForCurrentOffset() const;
    void SaveLastFramesCursorRow() const;
    void SavePokedexMainViewState() const;
    void CreatePokedexMainViewBackground();
    void DestroyPokedexMainViewBackground();

    static const glm::vec3 POKEDEX_MAIN_VIEW_BACKGROUND_POSITION;
    static const glm::vec3 POKEDEX_MAIN_VIEW_BACKGROUND_SCALE;
    static const glm::vec3 POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_POSITION;

    static const std::string POKEDEX_MAIN_VIEW_SPRITE_MODEL_FILE_NAME;
    static const std::string POKEDEX_MAIN_VIEW_BACKGROUND_TEXTURE_FILE_NAME;
    static const std::string TEXTBOX_CLICK_SFX_NAME;   

    static const int POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_COLS;
    static const int POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_ROWS;

    static const float POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_X_DISPLACEMENT;

    ecs::EntityId mPokedexMainViewBackgroundEntityId;
    ecs::EntityId mPokedexSelectionOptionsBareTextboxEntityId;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokedexMainViewOverworldFlowState_h */
