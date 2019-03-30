//
//  App.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef App_h
#define App_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ECS.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class App final
{
public:
    void Run();
    
private:
    void RegisterCommonComponents();
    void InitializeSystems();
    void GameLoop();
    bool AppShouldQuit();

private:
    ecs::World mWorld;    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* App_h */
