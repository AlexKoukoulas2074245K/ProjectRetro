//
//  NpcAiComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 03/05/2019.
//

#ifndef NpcAiComponent_h
#define NpcAiComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/GameConstants.h"
#include "../../common/utils/Timer.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class NpcAiComponent final: public ecs::IComponent
{
public:
    CharacterMovementType mMovementType = CharacterMovementType::DYNAMIC;
    std::unique_ptr<Timer> mAiTimer     = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


#endif /* NpcAiComponent_h */
