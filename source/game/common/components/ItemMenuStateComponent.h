//
//  ItemMenuStateComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 08/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef ItemMenuStateComponent_h
#define ItemMenuStateComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class ItemMenuStateComponent final: public ecs::IComponent
{
public:
    int mItemMenuOffsetFromStart              = 0;
    bool mShouldDisplayIndicationForMoreItems = false;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* ItemMenuStateComponent_h */
