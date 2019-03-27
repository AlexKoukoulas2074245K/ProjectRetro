//
//  Main.cpp
//  Hardcore2D
//
//  Created by Alex Koukoulas on 05/01/2019.
//

#include "ECS.h"

class DummyComponent: public IComponent
{
    
};

class Dummy2Component: public IComponent
{
    
};

class Dummy3Component: public IComponent
{
    
};

class BadComponent
{
    
};

class Dummy1System: public ISystem
{
public:
    void VUpdate(const float) override {}
};

class Dummy2System: public ISystem
{
public:
    void VUpdate(const float) override {}
};

int main(int, char**)
{
    World world;
    world.RegisterComponentType<DummyComponent>();
    world.RegisterComponentType<Dummy2Component>();
    world.RegisterComponentType<Dummy3Component>();
    
    world.RegisterSystem<DummyComponent, Dummy3Component>(std::make_unique<Dummy1System>());
    world.RegisterSystem<DummyComponent, Dummy2Component>(std::make_unique<Dummy2System>());
    return 0;
/*
    App app;
    
    if (app.Initialize())
    {
        app.Run();
    }
*/
}

