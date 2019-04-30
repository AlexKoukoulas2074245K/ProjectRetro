//
//  TransitionAnimationUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef TransitionAnimationUtils_h
#define TransitionAnimationUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../rendering/utils/Colors.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

inline glm::vec4 GetBackgroundColorBasedOnTransitionStep
(    
    const glm::vec4& currentLevelColor,
    const int transitionAnimationStep
)
{
    switch (transitionAnimationStep)
    {
        case 0: return currentLevelColor; 
        case 1: return colors::GLOBAL_BLUE_COLOR;
        case 2: return colors::GLOBAL_BLACK_COLOR;
        case 3: return colors::GLOBAL_BLACK_COLOR;
    }
    
    return currentLevelColor;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TransitionAnimationUtils_h */
