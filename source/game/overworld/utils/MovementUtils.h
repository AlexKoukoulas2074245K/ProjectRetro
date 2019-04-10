//
//  MovementUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef MovementUtils_h
#define MovementUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../common/GameConstants.h"
#include "../../common/components/TransformComponent.h"
#include "../../common/utils/MathUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class MoveOutcome
{
    IN_PROGRESS, COMPLETED
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MoveOutcome MoveToTargetPosition
(
    const glm::vec3& targetPosition,
    const float speed,
    const float dt,
    glm::vec3& currentPosition
)
{
    // Determine Axis of movement
    const auto movingOnXAxis = math::Abs(targetPosition.x - currentPosition.x) >
                               math::Abs(targetPosition.z - currentPosition.z);


    if (movingOnXAxis)
    {
        const auto movingRight = targetPosition.x - currentPosition.x > 0.0f;
        if (movingRight)
        {
            currentPosition.x += speed * dt;
        }
        else
        {
            currentPosition.x -= speed * dt;
        }

        return math::FloatsSufficientlyClose(currentPosition.x, targetPosition.x) ? MoveOutcome::COMPLETED : MoveOutcome::IN_PROGRESS;
    }
    else
    {
        const auto movingForward = targetPosition.z - currentPosition.z > 0.0f;
        if (movingForward)
        {
            currentPosition.z += speed * dt;
        }
        else
        {
            currentPosition.z -= speed * dt;
        }

        return math::FloatsSufficientlyClose(currentPosition.z, targetPosition.z) ? MoveOutcome::COMPLETED : MoveOutcome::IN_PROGRESS;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* MovementUtils_h */
