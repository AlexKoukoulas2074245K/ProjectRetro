//
//  AiInputControlStateSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 22/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef AiInputControlStateSingletonComponent_h
#define AiInputControlStateSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../common/utils/Timer.h"
#include "../../ECS.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class AiInputControllerType
{
    NONE, AI_OAK_PIKACHU_CAPTURE
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class AiInputControlStateSingletonComponent final: public ecs::IComponent
{
public:
    AiInputControllerType mAIInputControllerType        = AiInputControllerType::NONE;
    std::unique_ptr<Timer> mAiControllerInputDelayTimer = nullptr;
    int mAiControllerInputProgressionStep               = 0;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* AiInputControlStateSingletonComponent_h */