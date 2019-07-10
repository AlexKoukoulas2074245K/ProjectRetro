#//
//  PokemonBaseStatsSingletonComponent.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/07/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokemonBaseStatsSingletonComponent_h
#define PokemonBaseStatsSingletonComponent_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../GameConstants.h"
#include "../../ECS.h"
#include "../../common/utils/StringUtils.h"
#include "../../overworld/OverworldConstants.h"

#include <unordered_map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokemonBaseStats
{
public:
    PokemonBaseStats
    (
        std::vector<EvolutionInfo>& evolutions,
        const StringId name,
        const StringId firstType,
        const StringId secondType,
        const int id,
        const int hp,
        const int attack,
        const int defense,
        const int speed,
        const int special,
        const int xpStat,
        const int catchRate,
        const OverworldPokemonSpriteType overworldSpriteType
    )
        : mEvolutions(evolutions)
        , mName(name)
        , mFirstType(firstType)
        , mSecondType(secondType)
        , mId(id)
        , mHp(hp)
        , mAttack(attack)
        , mDefense(defense)
        , mSpeed(speed)
        , mSpecial(special)
        , mXpStat(xpStat)
        , mCatchRate(catchRate)
        , mOverworldSpriteType(overworldSpriteType)
    {
    }

    std::vector<EvolutionInfo> mEvolutions;
    const StringId mName;
    const StringId mFirstType;
    const StringId mSecondType;
    const int mId;
    const int mHp;
    const int mAttack;
    const int mDefense;
    const int mSpeed;
    const int mSpecial;
    const int mXpStat;
    const int mCatchRate;
    const OverworldPokemonSpriteType mOverworldSpriteType;    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokemonBaseStatsSingletonComponent final: public ecs::IComponent
{
public:
    std::unordered_map<StringId, PokemonBaseStats, StringIdHasher> mPokemonBaseStats;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokemonBaseStatsSingletonComponent_h */
