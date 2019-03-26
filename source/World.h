//
//  World.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 26/03/2019.
//

#ifndef World_h
#define World_h

#include <bitset>
#include <functional>

static constexpr int MAX_COMPONENTS = 64;

using ComponentMask = std::bitset<MAX_COMPONENTS>;

class ISystem final
{
    friend class World;
public:
    ISystem(const ISystem&) = delete;
    const ISystem& operator = (const ISystem&) = delete;
    
    virtual void VUpdate(const float dt) = 0;
    
protected:
    ISystem(const ComponentMask& componentUsageMask)
        : mComponentUsageMask(componentUsageMask)
    {}
    
    const ComponentMask mComponentUsageMask;
}

class World final
{
    
};

#endif /* World_h */
