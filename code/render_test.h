/*@H
* File: render_test.h
* Author: Jesse Calvert
* Created: October 19, 2018, 11:41
* Last modified: November 4, 2018, 16:10
*/

#pragma once

#include "cardcade_types.h"
#include "cardcade_shared.h"
#include "cardcade_platform.h"
#include "cardcade_debug_interface.h"
#include "cardcade_memory.h"
#include "cardcade_stream.h"
#include "cardcade_math.h"
#include "cardcade_random.h"

#include "cardcade_renderer.h"

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH

enum win32_memory_block_flags
{
	Win32Memory_AllocatedDuringLoop = 0x1,
	Win32Memory_DeallocatedDuringLoop = 0x2,
};
struct win32_memory_block
{
	platform_memory_block Block;

	win32_memory_block *Next;
	win32_memory_block *Prev;
	flag32(win32_memory_block_flags) LoopingFlags;
};
CTAssert(sizeof(win32_memory_block) == 64);

struct win32_state
{
	SYSTEM_INFO SystemInfo;

	char EXEFullPath[WIN32_STATE_FILE_NAME_COUNT];
	string EXEDirectory;

	ticket_mutex MemoryMutex;
	win32_memory_block MemorySentinel;

	b32 LoopRecording;
	b32 LoopPlayback;
	HANDLE RecordingHandle;
	HANDLE PlaybackHandle;
};

enum test_scene_element
{
	Element_Ground,
	Element_Wall,
	Element_Tree,

	Element_Count,
};
#define TEST_SCENE_DIM 32
struct test_scene
{
	test_scene_element Elements[TEST_SCENE_DIM][TEST_SCENE_DIM];

	renderer_texture Smile;
	renderer_texture Heart;
	renderer_texture Sword;

	camera Camera;
};

struct render_test_state
{
	memory_region Region;

	test_scene Scene;

	b32 Initialized;
};

internal renderer_texture LoadPNG(memory_region *Region, render_memory_op_queue *RenderMemoryOpQueue, string Filename, u32 Index);
