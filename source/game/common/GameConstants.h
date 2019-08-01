//
//  GameConstants.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 04/04/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef GameConstants_h
#define GameConstants_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "utils/StringUtils.h"

#include <array>
#include <memory>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float GAME_TILE_SIZE                 = 1.6f;
const float GUI_PIXEL_SIZE                 = 0.00859375f;
const float CHARACTER_ANIMATION_FRAME_TIME = 0.125f;
const float STATIONARY_NPC_RESET_TIME      = 1.5f;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum class Direction
{
    NORTH = 0, EAST, SOUTH, WEST
};

enum class CharacterMovementType
{
    DYNAMIC, STATIONARY, STATIC
};

enum class PokemonStatus
{
    NORMAL, PARALYZED, POISONED, CONFUSED, FROZEN, ASLEEP, BURNED
};

enum class OverworldPokemonSpriteType
{
    BALL, BEAST, BUG, DRAGON, FLYING, FOSSIL, GRASS, NORMAL, PIKACHU, WATER
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct PokemonMoveStats
{
    PokemonMoveStats
    (
        const StringId name,
        const StringId type,
        const StringId effect,
        const int power,
        const int accuracy,
        const int powerPoints
    )
        : mName(name)
        , mType(type)
        , mEffect(effect)
        , mPower(power)
        , mAccuracy(accuracy)
        , mTotalPowerPoints(powerPoints)
        , mPowerPointsLeft(powerPoints)
    {

    }

    const StringId mName;
    const StringId mType;
    const StringId mEffect;

    const int mPower;
    const int mAccuracy;
    
    int mTotalPowerPoints;
    int mPowerPointsLeft;
};

using PokemonMoveSet = std::array<std::unique_ptr<PokemonMoveStats>, 4>;

struct EvolutionInfo
{
    StringId mEvolutionTargetPokemonName;
    StringId mEvolutionMethod;
};

struct MoveLearnInfo
{
    StringId mMoveName;
    int mLevelLearned;
};

struct PokemonBaseStats
{
    PokemonBaseStats
    (
        std::vector<EvolutionInfo>& evolutions,
        std::vector<MoveLearnInfo>& learnset,
        const StringId speciesName,
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
        , mLearnset(learnset)
        , mSpeciesName(speciesName)
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

    const std::vector<EvolutionInfo> mEvolutions;
    const std::vector<MoveLearnInfo> mLearnset;
    const StringId mSpeciesName;
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

struct Pokemon
{
    Pokemon
    (
        const StringId name,
        const PokemonBaseStats& baseStats    
    )       
        : mName(name)
        , mBaseSpeciesStats(baseStats)             
    {
    }
    
    StringId mName;
    const PokemonBaseStats& mBaseSpeciesStats;    
    
    PokemonMoveSet mMoveSet;
    StringId mMoveToBeLearned;
    size_t mMovesetIndexForNewMove;
    
    int mLevel;
    int mXpPoints;

    PokemonStatus mStatus;

    // Current/Max stats
    int mHp;      int mMaxHp;                 
    int mAttack;  int mAttackEncounterStage;  
    int mDefense; int mDefenseEncounterStage;
    int mSpeed;   int mSpeedEncounterStage;
    int mSpecial; int mSpecialEncounterStage; 

    int mAccuracyStage;
    int mEvasionStage;

    // IVs/EVs
    int mHpIv;      int mHpEv;
    int mAttackIv;  int mAttackEv;
    int mDefenseIv; int mDefenseEv;
    int mSpeedIv;   int mSpeedEv;
    int mSpecialIv; int mSpecialEv;
};

struct TrainerInfo
{
    TrainerInfo
    (
        const StringId trainerSpeciesName,        
        const int basePayout,
        const int textureAtlasCol,
        const int textureAtlasRow
    )
        : mTrainerSpeciesName(trainerSpeciesName)
        , mBasePayout(basePayout)
        , mTextureAtlasCol(textureAtlasCol)
        , mTextureAtlasRow(textureAtlasRow)
    {
    }

    const StringId mTrainerSpeciesName;
    const int mBasePayout;
    const int mTextureAtlasCol;
    const int mTextureAtlasRow;
};

struct TileCoords
{
    TileCoords()
        : mCol(0)
        , mRow(0)
    {
    }

    TileCoords(const int col, const int row)
        : mCol(col)
        , mRow(row)
    {
    }

    int mCol;
    int mRow;
};

inline bool operator == (const TileCoords& lhs, const TileCoords& rhs)
{
    return lhs.mCol == rhs.mCol && lhs.mRow == rhs.mRow;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* GameConstants_h */
