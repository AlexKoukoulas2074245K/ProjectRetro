//
//  BaseEncounterFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/06/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef BaseEncounterFlowState_h
#define BaseEncounterFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../common/utils/Logging.h"

#include <memory>
#include <typeinfo>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

namespace ecs
{
    class World;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class BaseEncounterFlowState
{
    friend class EncounterStateControllerSystem;

public:
    BaseEncounterFlowState(ecs::World& world)
        : mWorld(world)
        , mNextFlowState(nullptr)
    {

    }

    virtual ~BaseEncounterFlowState() = default;
    
    virtual void VUpdate(const float dt) = 0;    

protected:
    template<class FlowStateType>
    void CompleteAndTransitionTo()
    {
        static_assert(std::is_base_of<BaseEncounterFlowState, FlowStateType>::value, 
            "Tried to transition to a non BaseEncounterFlowState subclass");
        
#ifndef NDEBUG
        Log(LogType::INFO, "Transitioned to state: %s", typeid(FlowStateType).name());
#endif
        
        mNextFlowState = std::make_unique<FlowStateType>(mWorld);

    }

    ecs::World& mWorld;

private:
    std::unique_ptr<BaseEncounterFlowState> mNextFlowState;

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* BaseEncounterFlowState_h */
