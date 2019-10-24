//
//  MilestoneAlterationsSystem.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 12/10/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MilestoneAlterationsSystem.h"
#include "../components/LevelModelComponent.h"
#include "../components/MilestoneAlterationTagComponent.h"
#include "../utils/LevelUtils.h"
#include "../utils/OverworldUtils.h"
#include "../../common/utils/MilestoneUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

namespace
{
    const StringId OAKS_LAB_LEVEL_NAME        = StringId("in_oaks_lab");
    const StringId PALLET_TOWN_LEVEL_NAME     = StringId("pallet_town");
    const StringId RIVALS_HOUSE_LEVEL_NAME    = StringId("in_rivals_home");
    const StringId VIRIDIAN_CITY_LEVEL_NAME   = StringId("viridian_city");
    const StringId ROUTE_22_LEVEL_NAME        = StringId("route_22");
    const StringId PEWTER_CITY_LEVEL_NAME     = StringId("pewter_city");
    const StringId PEWTER_CITY_GYM_LEVEL_NAME = StringId("in_pewter_gym");

    const TileCoords OAKS_LAB_FIRST_POKEDEX_COORDS                      = TileCoords(5, 11);
    const TileCoords OAKS_LAB_SECOND_POKEDEX_COORDS                     = TileCoords(6, 11);
    const TileCoords OAKS_LAB_POKEBALL_COORDS                           = TileCoords(10, 10);
    const TileCoords OAKS_LAB_MOVEMENT_AND_BATTLE_TRIGGER_1_TILE_COORDS = TileCoords(7, 6);
    const TileCoords OAKS_LAB_MOVEMENT_AND_BATTLE_TRIGGER_2_TILE_COORDS = TileCoords(8, 6);
    const TileCoords PALLET_TOWN_INTRO_SEQUENCE_TRIGGER_1_TILE_COORDS   = TileCoords(16,23);
    const TileCoords PALLET_TOWN_INTRO_SEQUENCE_TRIGGER_2_TILE_COORDS   = TileCoords(17, 23);
    const TileCoords VIRIDIAN_CITY_RUDE_GUY_TRIGGER_TILE_COORDS         = TileCoords(24, 34);
    const TileCoords ROUTE_22_RIVAL_BATTLE_TRIGGER_1_TILE_COORDS        = TileCoords(42, 20);
    const TileCoords ROUTE_22_RIVAL_BATTLE_TRIGGER_2_TILE_COORDS        = TileCoords(42, 19);
    const TileCoords PEWTER_CITY_BROCK_GUIDE_TRIGGER_1_TILE_COORDS      = TileCoords(46, 34);
    const TileCoords PEWTER_CITY_BROCK_GUIDE_TRIGGER_2_TILE_COORDS      = TileCoords(47, 33);
    const TileCoords PEWTER_CITY_BROCK_GUIDE_TRIGGER_3_TILE_COORDS      = TileCoords(48, 32);

    const int OAKS_LAB_FIRST_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX  = 4;
    const int OAKS_LAB_SECOND_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX = 5;
    const int OAKS_LAB_OAK_ENTITY_LEVEL_INDEX                       = 10;
    const int OAKS_LAB_GARY_ENTITY_LEVEL_INDEX                      = 11;
    const int OAKS_LAB_POKEBALL_ENTITY_LEVEL_INDEX                  = 12;
    const int RIVALS_HOME_SISTER_NPC_LEVEL_INDEX                    = 4;
    const int VIRIDIAN_RUDE_GUY_RELATIVE_LEVEL_INDEX                = 4;
    const int VIRIDIAN_RUDE_GUY_LEVEL_INDEX                         = 5;
    const int PEWTER_CITY_BROCK_GUIDE_LEVEL_INDEX                   = 2;
    const int PEWTER_CITY_GYM_TRAINER                               = 1;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MilestoneAlterationsSystem::MilestoneAlterationsSystem(ecs::World& world)
    : BaseSystem(world)
{
    CalculateAndSetComponentUsageMask<MilestoneAlterationTagComponent>();
}

void MilestoneAlterationsSystem::VUpdateAssociatedComponents(const float) const
{
    const auto& activeEntities = mWorld.GetActiveEntities();

    const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();    

    for (const auto& entityId : activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {                        
            auto& levelModelComponent = mWorld.GetComponent<LevelModelComponent>(entityId);
            const auto& levelName     = levelModelComponent.mLevelName;

            if (levelName == OAKS_LAB_LEVEL_NAME && HasMilestone(milestones::RECEIVED_POKEDEX, mWorld))
            {
                mWorld.DestroyEntity(FindEntityAtLevelCoords(OAKS_LAB_FIRST_POKEDEX_COORDS, mWorld));
                mWorld.DestroyEntity(FindEntityAtLevelCoords(OAKS_LAB_SECOND_POKEDEX_COORDS, mWorld));
                mWorld.DestroyEntity(FindEntityAtLevelCoords(OAKS_LAB_POKEBALL_COORDS, mWorld));

                DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(OAKS_LAB_FIRST_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX, mWorld), mWorld);
                DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(OAKS_LAB_SECOND_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX, mWorld), mWorld);
                DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(OAKS_LAB_POKEBALL_ENTITY_LEVEL_INDEX, mWorld), mWorld);                

                GetTile(OAKS_LAB_MOVEMENT_AND_BATTLE_TRIGGER_1_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
                GetTile(OAKS_LAB_MOVEMENT_AND_BATTLE_TRIGGER_1_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
            }   
            else if (levelName == OAKS_LAB_LEVEL_NAME && HasMilestone(milestones::FIRST_RIVAL_BATTLE_FINSIHED, mWorld))
            {
                mWorld.DestroyEntity(FindEntityAtLevelCoords(OAKS_LAB_POKEBALL_COORDS, mWorld));
                DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(OAKS_LAB_POKEBALL_ENTITY_LEVEL_INDEX, mWorld), mWorld);

                GetTile(OAKS_LAB_MOVEMENT_AND_BATTLE_TRIGGER_1_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
                GetTile(OAKS_LAB_MOVEMENT_AND_BATTLE_TRIGGER_1_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
            }
            else if (levelName == OAKS_LAB_LEVEL_NAME && HasMilestone(milestones::RECEIVED_PIKACHU, mWorld))
            {
                mWorld.DestroyEntity(FindEntityAtLevelCoords(OAKS_LAB_POKEBALL_COORDS, mWorld));
                DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(OAKS_LAB_POKEBALL_ENTITY_LEVEL_INDEX, mWorld), mWorld);

                auto& garyNpcAiComponent = mWorld.GetComponent<NpcAiComponent>(GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_ENTITY_LEVEL_INDEX, mWorld));
                garyNpcAiComponent.mDialog = playerStateComponent.mPlayerTrainerName.GetString() + ": Heh, my#POK^MON looks a#lot stronger.";
            }
            else if (levelName == OAKS_LAB_LEVEL_NAME && HasMilestone(milestones::SEEN_OAK_FIRST_TIME, mWorld))
            {                                
                auto& garyNpcAiComponent = mWorld.GetComponent<NpcAiComponent>(GetNpcEntityIdFromLevelIndex(OAKS_LAB_GARY_ENTITY_LEVEL_INDEX, mWorld));                
                garyNpcAiComponent.mDialog = playerStateComponent.mRivalName.GetString() + + ": Humph!#I'll get a better#POK^MON  than you!";
            }
            else if (levelName == OAKS_LAB_LEVEL_NAME && !HasMilestone(milestones::SEEN_OAK_FIRST_TIME, mWorld))
            {
                DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(OAKS_LAB_OAK_ENTITY_LEVEL_INDEX, mWorld), mWorld);

                GetTile(OAKS_LAB_MOVEMENT_AND_BATTLE_TRIGGER_1_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
                GetTile(OAKS_LAB_MOVEMENT_AND_BATTLE_TRIGGER_1_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
            }
            else if (levelName == PALLET_TOWN_LEVEL_NAME && HasMilestone(milestones::SEEN_OAK_FIRST_TIME, mWorld))
            {
                GetTile(PALLET_TOWN_INTRO_SEQUENCE_TRIGGER_1_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
                GetTile(PALLET_TOWN_INTRO_SEQUENCE_TRIGGER_2_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
            }
            else if (levelName == RIVALS_HOUSE_LEVEL_NAME && !HasMilestone(milestones::RECEIVED_POKEDEX, mWorld))
            {
                auto& rivalsSisterAiComponent = mWorld.GetComponent<NpcAiComponent>(GetNpcEntityIdFromLevelIndex(RIVALS_HOME_SISTER_NPC_LEVEL_INDEX, mWorld));
                rivalsSisterAiComponent.mDialog = "Hi PLAYERNAME!#" + playerStateComponent.mRivalName.GetString() + "#is out at#Grandpa's lab.";                
            }
            else if (levelName == VIRIDIAN_CITY_LEVEL_NAME && HasMilestone(milestones::RECEIVED_POKEDEX, mWorld))
            {                
                auto& rudeGuyRelativeAiComponent   = mWorld.GetComponent<NpcAiComponent>(GetNpcEntityIdFromLevelIndex(VIRIDIAN_RUDE_GUY_RELATIVE_LEVEL_INDEX, mWorld));
                rudeGuyRelativeAiComponent.mDialog = "When I go shop in#PEWTER CITY, I#have to take the#winding trail in#VIRIDIAN FOREST.";
                GetTile(VIRIDIAN_CITY_RUDE_GUY_TRIGGER_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
                DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(VIRIDIAN_RUDE_GUY_LEVEL_INDEX, mWorld), mWorld);
            }
            else if (levelName == ROUTE_22_LEVEL_NAME && HasMilestone(milestones::SECOND_RIVAL_BATTLE_WON, mWorld))
            {
                GetTile(ROUTE_22_RIVAL_BATTLE_TRIGGER_1_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
                GetTile(ROUTE_22_RIVAL_BATTLE_TRIGGER_2_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
            }
            else if (levelName == PEWTER_CITY_LEVEL_NAME && HasMilestone(milestones::BOULDERBADGE, mWorld))
            {
                DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(PEWTER_CITY_BROCK_GUIDE_LEVEL_INDEX, mWorld), mWorld);
                GetTile(PEWTER_CITY_BROCK_GUIDE_TRIGGER_1_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
                GetTile(PEWTER_CITY_BROCK_GUIDE_TRIGGER_2_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
                GetTile(PEWTER_CITY_BROCK_GUIDE_TRIGGER_3_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
            }
            else if (levelName == PEWTER_CITY_GYM_LEVEL_NAME && HasMilestone(milestones::BOULDERBADGE, mWorld))
            {
                auto& pewterGymTrainerAiComponent = mWorld.GetComponent<NpcAiComponent>(GetNpcEntityIdFromLevelIndex(PEWTER_CITY_GYM_TRAINER, mWorld));
                pewterGymTrainerAiComponent.mIsDefeated = true;
                pewterGymTrainerAiComponent.mDialog = pewterGymTrainerAiComponent.mSideDialogs[1];
            }
            
            mWorld.RemoveComponent<MilestoneAlterationTagComponent>(entityId);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

