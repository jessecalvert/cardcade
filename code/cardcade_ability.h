/*@H
* File: cardcade_ability.h
* Author: Jesse Calvert
* Created: November 6, 2018, 16:23
* Last modified: December 17, 2018, 14:39
*/

#pragma once

INTROSPECT()
enum ability_type
{
	AbilityType_Passive,
	AbilityType_Reaction,
	AbilityType_Activated,
};

struct ability_cost
{
	s32 Charges;
	s32 Gold;
	u32 Actions;
};

enum target_flags
{
	Target_OnBoard = (1 << 0),
	Target_InRange = (1 << 1),
	Target_Traversable = (1 << 2),
	Target_InHand = (1 << 3),
	Target_Influenced = (1 << 4),
	Target_Building = (1 << 5),
	Target_Unit = (1 << 6),
};

INTROSPECT()
enum reaction_trigger
{
	Reaction_TurnStart,
	Reaction_TurnEnd,
	Reaction_OnDestroy,
	Reaction_OnChargeChange,
};

struct ability
{
	ability_name Name;
	ability_type Type;
	string DisplayName;
	char *DescriptionFormat;

	s32 Parameters[8];

	// NOTE: Activated
	ability_cost Cost;
	s32 Cooldown;
	s32 CurrentCooldown;
	u32 TargetCount;
	flag32(target_flags) TargetFlags;
	s32 Range;

	// NOTE: Reaction
	reaction_trigger Trigger;

	// NOTE: Passive

};

internal b32 UseAbility(game_state *GameState, struct game_mode_battle *BattleMode,
	entity *Player, entity *CastingEntity, ability *Ability, entity_id *TargetIDs);
