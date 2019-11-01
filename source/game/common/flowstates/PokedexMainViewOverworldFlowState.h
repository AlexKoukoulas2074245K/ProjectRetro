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
#include "../utils/StringUtils.h"
#include "../utils/Timer.h"
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
    enum class PokedexMainViewListActionType
    {
        SCROLL_UP, SCROLL_DOWN, PAGE_UP, PAGE_DOWN, SELECT_ENTRY
    };

    static const glm::vec3 POKEDEX_MAIN_VIEW_BACKGROUND_POSITION;
    static const glm::vec3 POKEDEX_MAIN_VIEW_BACKGROUND_SCALE;
    static const glm::vec3 POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_POSITION;

    static const std::string POKEDEX_MAIN_VIEW_SPRITE_MODEL_FILE_NAME;
    static const std::string POKEDEX_MAIN_VIEW_BACKGROUND_TEXTURE_FILE_NAME;
    static const std::string TEXTBOX_CLICK_SFX_NAME;

    static const int POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_COLS;
    static const int POKEDEX_SELECTION_OPTIONS_BARE_TEXTBOX_ROWS;

    static const float POKEDEX_RAPID_SCROLL_ENABLING_TIMER_DELAY;
    static const float POKEDEX_RAPID_SCROLL_ADVANCE_TIMER_DELAY;

    void UpdateMainView(const float dt);    
    void UpdateSelectionView(const float dt);
    void CancelPokedexMainView();    
    void RedrawPokedexMainView() const;
    void DisplayPokedexEntriesForCurrentOffset() const;
    void SaveLastFramesCursorRow() const;    
    void CreatePokedexMainViewBackground();    
    void DoActionInPokedexMainViewPokemonList(const PokedexMainViewListActionType) const;
        
    const int mMaxSeenOrOwnedPokemonId;
    int mSelectedPokemonId;

    Timer mPokedexRapidScrollEnablingTimer;
    Timer mPokedexRapidScrollAdvanceTimer;    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokedexMainViewOverworldFlowState_h */
