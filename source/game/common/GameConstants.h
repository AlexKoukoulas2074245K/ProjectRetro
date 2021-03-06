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
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const float GAME_TILE_SIZE                 = 1.6f;
const float GUI_PIXEL_SIZE                 = 0.00859375f;
const float CHARACTER_ANIMATION_FRAME_TIME = 0.125f;
const float STATIONARY_NPC_RESET_TIME      = 1.5f;

static const StringId CANCEL_ITEM_NAME      = StringId("CANCEL");
static const StringId OAKS_PARCEL_ITEM_NAME = StringId("OAK's_PARCEL");
static const StringId POKEDEX_ITEM_NAME     = StringId("POK^DEX");

static constexpr int MAX_ROSTER_SIZE              = 6;
static constexpr int MAX_POKEMON_ID               = 151;
static constexpr int PLAYER_STARTING_POKE_DOLLARS = 3000;
static constexpr int MAX_TRAINER_ID               = 65535;

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

enum class ItemUsageType
{
    UNUSABLE, OVERWORLD, BATTLE, ANYWHERE
};

enum class PokemonStatus
{
    NORMAL, PARALYZED, POISONED, CONFUSED, FROZEN, ASLEEP, BURNED
};

enum class PokemonType
{
    NORMAL, PSYCHIC, ELECTRIC, FLYING, GRASS, WATER, ROCK, GROUND, ICE, FIRE, GHOST, BUG, POISON, DRAGON
};

enum class OverworldPokemonSpriteType
{
    BALL, BEAST, BUG, DRAGON, FLYING, FOSSIL, GRASS, NORMAL, PIKACHU, WATER
};

enum ItemDiscoveryType
{
    NO_ITEM, FOUND, GOT, RECEIVED, DELIVERED, SNATCHED
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::unordered_map<PokemonType, StringId> sPokemonTypesToStrings =
{
    { PokemonType::NORMAL, StringId("NORMAL")},
    { PokemonType::PSYCHIC, StringId("PSYCHIC") },
    { PokemonType::ELECTRIC, StringId("ELECTRIC") },
    { PokemonType::FLYING, StringId("FLYING") },
    { PokemonType::GRASS, StringId("GRASS") },
    { PokemonType::WATER, StringId("WATER") },
    { PokemonType::ROCK, StringId("ROCK") },
    { PokemonType::GROUND, StringId("GROUND") },
    { PokemonType::ICE, StringId("ICE") },
    { PokemonType::FIRE, StringId("FIRE") },
    { PokemonType::GHOST, StringId("GHOST") },
    { PokemonType::BUG, StringId("BUG") },
    { PokemonType::POISON, StringId("POISON") },
    { PokemonType::DRAGON, StringId("DRAGON") },
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct ItemStats
{
    ItemStats
    (
        const StringId name,
        const StringId effect,
        const int price,
        const ItemUsageType usageType,
        const bool unique
    )
        : mName(name)
        , mEffect(effect)
        , mPrice(price)
        , mUsageType(usageType)
        , mUnique(unique)
    {
    }

    const StringId mName;
    const StringId mEffect;
    const int mPrice;
    const ItemUsageType mUsageType;
    const bool mUnique;
};

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
        const std::string& pokedexText,
        const std::string& pokedexPokemonType,
        const std::string& pokedexBodyStats,
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
        , mPokedexText(pokedexText)
        , mPokedexPokemonType(pokedexPokemonType)
        , mPokedexBodyStats(pokedexBodyStats)
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
    std::string mPokedexText;
    std::string mPokedexPokemonType;
    std::string mPokedexBodyStats;
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
    
    std::unique_ptr<Pokemon> mEvolution;

    PokemonMoveSet mMoveSet;
    StringId mMoveToBeLearned;
    size_t mMovesetIndexForNewMove;
    
    int mLevel;
    int mXpPoints;
    
    PokemonStatus mStatus;

    int mNumberOfRoundsUntilConfusionEnds;
    int mNumberOfRoundsUntilSleepEnds;
    
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

    // Move-specific attributes
    int mBideCounter;
    int mBideAccumulatedDamage;
    int mBindingOrWrappingOpponentCounter;
    int mBindingOrWrappingContinuationDamage;
};

struct TrainerInfo
{
    TrainerInfo
    (
        const StringId trainerSpeciesName,
        const StringId trainerMusicTrackName,
        const int basePayout,
        const int textureAtlasCol,
        const int textureAtlasRow
    )
        : mTrainerSpeciesName(trainerSpeciesName)
        , mTrainerMusicTrackName(trainerMusicTrackName)
        , mBasePayout(basePayout)
        , mTextureAtlasCol(textureAtlasCol)
        , mTextureAtlasRow(textureAtlasRow)
    {
    }

    const StringId mTrainerSpeciesName;
    const StringId mTrainerMusicTrackName;
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
