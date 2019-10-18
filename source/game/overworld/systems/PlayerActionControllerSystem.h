//
//  PlayerActionControllerSystem.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 01/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PlayerActionControllerSystem_h
#define PlayerActionControllerSystem_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/GameConstants.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class DirectionComponent;
class MovementStateComponent;
class RenderableComponent;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PlayerActionControllerSystem final: public ecs::BaseSystem
{
public:
    PlayerActionControllerSystem(ecs::World&);
    
    void VUpdateAssociatedComponents(const float dt) const override;

private:
    void StartPendingPostEncounterConversation() const;
    void AddPendingItemsToBag() const;
    
    void CheckForNpcInteraction
    (
        const Direction,
        const MovementStateComponent&
    ) const;
    
    void ChangePlayerDirectionAndAnimation
    (
        const Direction,
        RenderableComponent&,
        DirectionComponent&
    ) const;
    
    static const std::string MENU_OPEN_SFX_NAME;    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PlayerActionControllerSystem_h */
