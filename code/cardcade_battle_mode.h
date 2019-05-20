/*@H
* File: cardcade_battle_mode.h
* Author: Jesse Calvert
* Created: May 21, 2018, 14:48
* Last modified: December 17, 2018, 14:59
*/

#pragma once

INTROSPECT()
enum player_name
{
	Player_None,
	Player_Neutral,
	Player_1,
	Player_2,
	Player_Count,
};

struct entity_list
{
	u32 EntityCount;
	entity_id EntityIDs[16];
};

struct board_tile
{
	v2i BoardPosition;
	v3 P;
	v3 Dim;

	entity_list List;
};

struct board
{
	u32 Width;
	u32 Height;
	board_tile *Tiles;
};

INTROSPECT()
enum game_phase
{
	Phase_None,

	Phase_GameBegin,
	Phase_PlaceHQ,
	Phase_HQEnd,

	Phase_TurnStart,
	Phase_PlayerActions,
	Phase_TurnEnd,

	Phase_GameOver,
};

struct ui_panel_positions
{
	v2i At;
	v2i Advance;
	v2i Dim;
	s32 Spacing;
	v2i LeftCenterP;
};

struct hovered_info
{
	entity_id ID;
	ability *Ability;
};

struct deck
{
	u32 CardCount;
	ability_name Cards[128];
};

struct game_mode_battle
{
	physics_state Physics;
	particle_system *ParticleSystem;
	animator Animator;
	struct entity_manager *EntityManager;

	random_series Entropy;

	f32 Pitch;
	f32 Yaw;
	camera WorldCamera;
	camera ScreenCamera;

	u32 DecalCount;
	oriented_box Decals[256];

	u32 ActionsPerTurn;
	s32 PointsToWin;
	player_name CurrentPlayer;
	entity_id Players[Player_Count];
	deck Decks[Player_Count];
	u32 CurrentTurn;

	ability Abilities[Ability_Count];

	game_phase Phase;
	b32 GameOver_;

	board Board;
	v3 GroundDim;
	v3 CardDim;
	f32 GroundSpacing;

	entity_id HighlightID;
	ui_state UI;
};

internal void PlayBattleMode(game_state *GameState);
