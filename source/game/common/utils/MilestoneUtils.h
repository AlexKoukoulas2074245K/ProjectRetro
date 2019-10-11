//
//  MilestoneUtils.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 11/10/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef MilestoneUtils_h
#define MilestoneUtils_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../../ECS.h"
#include "../components/PlayerStateSingletonComponent.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

inline void SetMilestone(const unsigned long mileStone, ecs::World& world)
{
	auto& playerStateComponent = world.GetSingletonComponent<PlayerStateSingletonComponent>();
	playerStateComponent.mMilestones |= mileStone;
}

inline void UnsetMilestone(const unsigned long mileStone, ecs::World& world)
{
	auto& playerStateComponent = world.GetSingletonComponent<PlayerStateSingletonComponent>();
	playerStateComponent.mMilestones ^= mileStone;
}

inline bool HasMilestone(const unsigned long mileStone, const ecs::World& world)
{
	const auto& playerStateComponent = world.GetSingletonComponent<PlayerStateSingletonComponent>();
	return (playerStateComponent.mMilestones & mileStone) != 0;
}

inline int GetNumberOfBadges(const ecs::World& world)
{
	int badgeCount = 0;

	if (HasMilestone(milestones::BOULDERBADGE, world)) badgeCount++;
	if (HasMilestone(milestones::CASCADEBADGE, world)) badgeCount++;
	if (HasMilestone(milestones::THUNDERBADGE, world)) badgeCount++;
	if (HasMilestone(milestones::RAINBOWBADGE, world)) badgeCount++;
	if (HasMilestone(milestones::SOULBADGE, world))    badgeCount++;
	if (HasMilestone(milestones::MARSHBADGE, world))   badgeCount++;
	if (HasMilestone(milestones::VOLCANOBADGE, world)) badgeCount++;
	if (HasMilestone(milestones::EARTHBADGE, world))   badgeCount++;

	return badgeCount;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* MilestoneUtils_h */
