//
//  TownMapLocationDataSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 25/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef TownMapLocationDataSingletonComponent_h
#define TownMapLocationDataSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../../common/GameConstants.h"
#include "../../common/utils/StringUtils.h"
#include "../../common/utils/MathUtils.h"

#include <unordered_map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct TownMapLocationEntry
{
    TownMapLocationEntry(const StringId location, const glm::vec3& position)
        : mLocation(location)
        , mPosition(position)
    {
    }

    const StringId mLocation;
    const glm::vec3 mPosition;
};

class TownMapLocationDataSingletonComponent final: public ecs::IComponent
{
public:    
    std::unordered_map<StringId, StringId, StringIdHasher> mIndoorLocationsToOwnerLocations;
    std::vector<TownMapLocationEntry> mTownMapLocations;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TownMapLocationDataSingletonComponent_h */
