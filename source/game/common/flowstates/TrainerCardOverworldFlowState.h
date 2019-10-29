//
//  TrainerCardOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef TrainerCardOverworldFlowState_h
#define TrainerCardOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseOverworldFlowState.h"
#include "../GameConstants.h"
#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"
#include "../../ECS.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class TrainerCardOverworldFlowState final: public BaseOverworldFlowState
{
public:
    TrainerCardOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    void CreateBackground();
    void CreateInfoTextbox();
    void CreateEmblems();
    void DestroyBackground();
    void DestroyInfoTextbox();
    void DestroyEmblems();

    static const std::string TRAINER_BACKGROUND_SPRITE_MODEL_FILE_NAME;
    static const std::string TRAINER_BACKGROUND_TEXTURE_FILE_NAME;
    static const std::string ENCOUNTER_SPRITE_MODEL_NAME;
    static const std::string ENCOUNTER_SPRITE_ANIMATION_NAME;
    static const std::string ENCOUNTER_SPRITE_SHADER_NAME;
    static const std::string EMBLEMS_ATLAS_FILE_NAME;

    static const glm::vec3 BACKGROUND_POSITION;
    static const glm::vec3 BACKGROUND_SCALE;
    static const glm::vec3 TRAINER_CARD_BACKGROUND_POSITION;    
    static const glm::vec3 TRAINER_CARD_BACKGROUND_SCALE;
    static const glm::vec3 INFO_TEXTBOX_POSITION;
    static const glm::vec3 EMBLEM_SPRITE_SCALE;
    static const glm::vec3 EMBLEM_SPRITE_ORIGIN_POSITION;

    static const int INFO_TEXTBOX_COLS;
    static const int INFO_TEXTBOX_ROWS;
    static const int EMBLEM_ATALS_COLS;
    static const int EMBLEM_ATALS_ROWS;

    static const float EMBLEMS_HOR_DISTANCE;
    static const float EMBLEMS_VER_DISTANCE;

    std::vector<ecs::EntityId> mEmblemEntityIds;
    ecs::EntityId mBackgroundCoverEntityId;
    ecs::EntityId mInfoTextboxEntityId;
    ecs::EntityId mTrainerCardBackgroundEntityId;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TrainerCardOverworldFlowState_h */
