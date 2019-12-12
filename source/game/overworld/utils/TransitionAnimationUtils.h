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

#include "LevelUtils.h"
#include "../../ECS.h"
#include "../../rendering/utils/Colors.h"
#include "../../common/utils/StringUtils.h"
#include "../../overworld/components/TransitionAnimationStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

inline glm::vec4 GetBackgroundColorBasedOnContext
(    
    const glm::vec4& currentLevelColor,
    const StringId currentLevelNameId,
    const TransitionAnimationStateSingletonComponent& transitionStateAnimationComponent
)
{
    if (currentLevelNameId.GetString() == "battle")
    {
        if 
        (
            transitionStateAnimationComponent.mDarkFlipProgressionStep == 1 ||
            transitionStateAnimationComponent.mDarkFlipProgressionStep == 2 ||
            transitionStateAnimationComponent.mWhiteFlipProgressionStep == 2
        )
        {
            return colors::GLOBAL_BLACK_COLOR;
        }

        return colors::GLOBAL_WHITE_COLOR;
    }
    else if (currentLevelNameId.GetString() == "intro")
    {
        return colors::GLOBAL_WHITE_COLOR;
    }
    
    if (IsLevelIndoors(currentLevelNameId))
    {
        return colors::GLOBAL_BLACK_COLOR;
    }

    switch (transitionStateAnimationComponent.mAnimationProgressionStep)
    {
        case -4:
        case -3:
        case -2:
        case -1: return colors::GLOBAL_WHITE_COLOR;
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
