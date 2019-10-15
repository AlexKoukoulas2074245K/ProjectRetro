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
	const StringId OAKS_LAB_LEVEL_NAME      = StringId("in_oaks_lab");
	const StringId RIVALS_HOUSE_LEVEL_NAME  = StringId("in_rivals_home");
	const StringId VIRIDIAN_CITY_LEVEL_NAME = StringId("viridian_city");
	const StringId ROUTE_22_LEVEL_NAME      = StringId("route_22");

	const TileCoords OAKS_LAB_FIRST_POKEDEX_COORDS               = TileCoords(5, 11);
	const TileCoords OAKS_LAB_SECOND_POKEDEX_COORDS              = TileCoords(6, 11);
	const TileCoords VIRIDIAN_CITY_RUDE_GUY_TRIGGER_TILE_COORDS  = TileCoords(24, 34);
	const TileCoords ROUTE_22_RIVAL_BATTLE_TRIGGER_1_TILE_COORDS = TileCoords(42, 20);
	const TileCoords ROUTE_22_RIVAL_BATTLE_TRIGGER_2_TILE_COORDS = TileCoords(42, 19);

	const int OAKS_LAB_FIRST_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX  = 4;
	const int OAKS_LAB_SECOND_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX = 5;
	const int RIVALS_HOME_SISTER_NPC_LEVEL_INDEX                    = 4;
	const int VIRIDIAN_RUDE_GUY_RELATIVE_LEVEL_INDEX                = 4;
	const int VIRIDIAN_RUDE_GUY_LEVEL_INDEX                         = 5;
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
				DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(OAKS_LAB_FIRST_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX, mWorld), mWorld);
				DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(OAKS_LAB_SECOND_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX, mWorld), mWorld);
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
			else if (levelName == ROUTE_22_LEVEL_NAME && HasMilestone(milestones::FIRST_RIVAL_BATTLE_WON, mWorld))
			{
				GetTile(ROUTE_22_RIVAL_BATTLE_TRIGGER_1_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
				GetTile(ROUTE_22_RIVAL_BATTLE_TRIGGER_2_TILE_COORDS, levelModelComponent.mLevelTilemap).mTileTrait = TileTrait::NONE;
			}
            
            mWorld.RemoveComponent<MilestoneAlterationTagComponent>(entityId);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

