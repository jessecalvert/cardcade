/*@H
* File: cardcade.h
* Author: Jesse Calvert
* Created: November 6, 2017
* Last modified: December 17, 2018, 17:08
*/

#pragma once

#include "cardcade_types.h"
#include "cardcade_shared.h"
#include "cardcade_platform.h"
#include "cardcade_debug_interface.h"

#include "cardcade_intrinsics.h"
#include "cardcade_simd.h"
#include "cardcade_math.h"
#include "cardcade_memory.h"
#include "cardcade_stream.h"

#include "cardcade_renderer.h"
#include "cardcade_config.h"
#include "cardcade_file_formats.h"
#include "cardcade_assets.h"

#include "cardcade_sort.h"
#include "cardcade_random.h"

#define COLOR_IRON V3(0.56f, 0.57f, 0.58f)
#define COLOR_GOLD V3(1.0f, 0.71f, 0.29f)
#define COLOR_COPPER V3(0.95f, 0.64f, 0.54f)
#define COLOR_ALUMINIUM V3(0.91f, 0.92f, 0.92f)
#define COLOR_SILVER V3(0.95f, 0.93f, 0.88f)

struct task_memory
{
	b32 InUse;

	memory_region Region;
	temporary_memory MemoryFlush;
};

internal task_memory *BeginTaskWithMemory(game_state *TranState);
internal void EndTaskWithMemory(task_memory *TaskMemory);

#include "cardcade_aabb.h"
#include "cardcade_gjk.h"
#include "cardcade_rigid_body.h"
#include "cardcade_convex_hull.h"
#include "cardcade_animation.h"
#include "cardcade_particles.h"
#include "cardcade_ui.h"

#include "cardcade_ability.h"
#include "cardcade_battle_mode.h"
#include "cardcade_entity_manager.h"
#include "cardcade_title_screen.h"
#include "cardcade_editor.h"

enum game_mode
{
	GameMode_None,

	GameMode_BattleMode,
	GameMode_TitleScreen,

	GameMode_Count,
};

struct console_entry
{
	console_entry *Next;
	console_entry *Prev;

	string String;
	v4 Color;
};

struct console
{
	b32 Expanded;
	stream *Info;
	console_entry Sentinel;
};

enum dev_mode
{
	DevMode_None,
	DevMode_Editor,
};

struct game_state
{
	memory_region Region;
	memory_region ModeRegion;

	temporary_memory FrameRegionTemp;
	memory_region FrameRegion;

	task_memory Tasks[32];

	struct assets *Assets;

	platform_work_queue *HighPriorityQueue;
	platform_work_queue *LowPriorityQueue;

	f32 TimeScale;

	game_mode CurrentMode;
	union
	{
		game_mode_battle *BattleMode;
		game_mode_title_screen *TitleScreen;
	};

	ui_state UI;
	dev_mode DevMode;
	b32 UseDebugCamera;
	console Console;
	game_editor *Editor;
};

internal void SetGameMode(game_state *GameState, game_mode Mode);

#if GAME_DEBUG
#include "generated.h"
#endif
