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
#include "../../common/utils/MilestoneUtils.h"
#include "../utils/OverworldUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

namespace
{
	const TileCoords OAKS_LAB_FIRST_POKEDEX_COORDS  = TileCoords(5, 11);
	const TileCoords OAKS_LAB_SECOND_POKEDEX_COORDS = TileCoords(6, 11);

	const int OAKS_LAB_FIRST_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX  = 4;
	const int OAKS_LAB_SECOND_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX = 5;
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
    for (const auto& entityId : activeEntities)
    {
        if (ShouldProcessEntity(entityId))
        {
            const auto& levelModelComponent = mWorld.GetComponent<LevelModelComponent>(entityId);
            const auto& levelName           = levelModelComponent.mLevelName;
            
            if (levelName == StringId("in_oaks_lab") && HasMilestone(milestones::RECEIVED_POKEDEX, mWorld))
            {
				mWorld.DestroyEntity(FindEntityAtLevelCoords(OAKS_LAB_FIRST_POKEDEX_COORDS, mWorld));
				mWorld.DestroyEntity(FindEntityAtLevelCoords(OAKS_LAB_SECOND_POKEDEX_COORDS, mWorld));
				DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(OAKS_LAB_FIRST_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX, mWorld), mWorld);
				DestroyOverworldNpcEntityAndEraseTileInfo(GetNpcEntityIdFromLevelIndex(OAKS_LAB_SECOND_POKEDEX_NPC_HIDDEN_ENTITY_LEVEL_INDEX, mWorld), mWorld);
            }
            
            mWorld.RemoveComponent<MilestoneAlterationTagComponent>(entityId);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

