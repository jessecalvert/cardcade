/*@H
* File: cardcade_renderer.h
* Author: Jesse Calvert
* Created: October 23, 2018, 15:14
* Last modified: November 6, 2018, 12:51
*/

#pragma once

#define MAX_TEXTURE_DIM 1024
#define MAX_LIGHTS 16
#define MAX_DECALS 16

INTROSPECT()
enum shader_type
{
	Shader_Null,
	Shader_Header,

	Shader_Scene_Quads,
	Shader_Scene_Quads_SDF,
	Shader_Scene_Meshes,
	Shader_ResolveDepthPeel,
	Shader_CompositeDepthPeel,
	Shader_Overlay,
	Shader_Overlay_SDF,
	Shader_Overbright,
	Shader_Blur,
	Shader_Finalize,

	Shader_DebugView,

	Shader_Count,
};

struct renderer_texture
{
	u32 Index;
	u16 Width;
	u16 Height;
};

struct renderer_mesh
{
	u32 Index;
	u16 VertexCount;
	u16 FaceCount;
};

INTROSPECT()
struct render_settings
{
	v2i Resolution;
	u32 SwapInterval;
	f32 RenderScale;

	b32 Bloom;
	u32 DesiredDepthPeelCount;
	u32 DesiredMultisamples;
};
inline b32
AreEqual(render_settings *A, render_settings *B)
{
	b32 Result =
		((A->Resolution.x == B->Resolution.x) &&
		 (A->Resolution.y == B->Resolution.y) &&
		 (A->RenderScale == B->RenderScale) &&
		 (A->SwapInterval == B->SwapInterval) &&
		 (A->Bloom == B->Bloom) &&
		 (A->DesiredDepthPeelCount == B->DesiredDepthPeelCount) &&
		 (A->DesiredMultisamples == B->DesiredMultisamples)
		 );
	return Result;
}

struct platform_renderer;
struct renderer_memory;
struct render_memory_op_queue;

#define RENDERER_PROCESS_MEMORY_OP_QUEUE(Name) void Name(platform_renderer *Renderer, render_memory_op_queue *RenderOpQueue)
typedef RENDERER_PROCESS_MEMORY_OP_QUEUE(renderer_process_memory_op_queue);

#define RENDERER_BEGIN_FRAME(Name) game_render_commands *Name(platform_renderer *Renderer, render_settings RenderSettings, stream *Info)
typedef RENDERER_BEGIN_FRAME(renderer_begin_frame);

#define RENDERER_END_FRAME(Name) void Name(platform_renderer *Renderer)
typedef RENDERER_END_FRAME(renderer_end_frame);

struct platform_renderer
{
	renderer_process_memory_op_queue *ProcessMemoryOpQueue;
	renderer_begin_frame *BeginFrame;
	renderer_end_frame *EndFrame;

	// NOTE: Supplied from the platform layer.
	platform *Platform;

#if GAME_DEBUG
	debug_table *DebugTable;
#endif
};

enum render_memory_op_type
{
	RenderOp_LoadMesh,
	RenderOp_LoadTexture,
};

struct asset;
struct loaded_mesh;
struct loaded_bitmap;

struct load_mesh_op
{
	renderer_mesh Mesh;
	void *VertexMemory;
	void *FaceMemory;
};

struct load_texture_op
{
	renderer_texture Texture;
	void *Pixels;
};

struct render_memory_op
{
	render_memory_op_type Type;
	union
	{
		load_mesh_op LoadMesh;
		load_texture_op LoadTexture;
	};

	render_memory_op *Next;
};

struct render_memory_op_queue
{
	render_memory_op *First;
	render_memory_op *Last;
	render_memory_op *FirstFree;

	ticket_mutex Mutex;
};

struct vertex_format
{
	v3 Position;
	v3 Normal;
	v2 TexCoords;
	u32 Color;
	u32 TextureIndex;
	v2 Roughness_Metalness;
};

struct surface_material
{
	f32 Roughness;
	f32 Metalness;
	v4 Color;
};

struct render_command_mesh_data
{
	mat4 Model;
	mat3 NormalMatrix;
	surface_material Material;
	u32 TextureIndex;
	u32 MeshIndex;
};

struct directional_light
{
	v3 Color;
	f32 Intensity;
	v3 Direction;
	f32 ArcDegrees;
};

struct point_light
{
	v3 Color;
	f32 Intensity;
	v3 P;
	f32 Size;
	f32 Radius;
	f32 LinearAttenuation;
	f32 QuadraticAttenuation;
};

struct ambient_light
{
	f32 Coeff;
	v3 Color;
};

enum light_type
{
	LightType_Point,
	LightType_Directional,
	LightType_Ambient,

	LightType_Count,
};
struct light
{
	light_type Type;
	union
	{
		directional_light Directional;
		point_light Point;
		ambient_light Ambient;
	};
};

struct decal
{
	oriented_box Box;
	renderer_texture TextureHandle;;
};

struct game_render_commands
{
	render_settings Settings;

	b32 InUse;
	renderer_texture WhiteTexture;

	u32 Size;
	u32 MaxSize;
	u8 *Buffer;

	// NOTE Quad data:
	u32 MaxVertexCount;
	u32 VertexCount;
	vertex_format *VertexArray;

	u32 MaxIndexCount;
	u32 IndexCount;
	u32 *IndexArray;

	// NOTE: Mesh data:
	u32 MaxMeshCount;
	u32 MeshCount;
	render_command_mesh_data *MeshArray;

	// NOTE: Light/decal data.
	u32 LightCount;
	light Lights[MAX_LIGHTS];

	u32 DecalCount;
	decal Decals[MAX_DECALS];
};

enum render_command_type
{
	RenderCommandType_render_entry_quads,
	RenderCommandType_render_entry_meshes,
	RenderCommandType_render_entry_begin_depth_peels,
	RenderCommandType_render_entry_end_depth_peels,
	RenderCommandType_render_entry_full_clear,
	RenderCommandType_render_entry_clear_depth,
	RenderCommandType_render_entry_set_target,
};

struct render_entry_full_clear
{
	v4 Color;
};

struct render_entry_header
{
	render_command_type Type;
};

enum render_setup_flags
{
	RenderFlag_TopMost = 0x1,
	RenderFlag_AlphaBlended = 0x2,
	RenderFlag_NotLighted = 0x4,
	RenderFlag_SDF = 0x8,
};

enum render_target_type
{
	Target_Scene,
	Target_Overlay,
	Target_Count,
};

struct render_entry_set_target
{
	render_target_type Target;
};

struct render_setup
{
	flag32(render_setup_flags) Flags;
	render_target_type Target;

	mat4 Projection;
	mat4 View;
	mat4 InvView;

	f32 FarPlane;
	v3 ViewRay;
	v3 P;
	quaternion CameraRotation;

	v2i ClipRectMin;
	v2i ClipRectMax;
};

struct render_entry_quads
{
	render_setup Setup;

	u32 QuadCount;
	u32 VertexArrayOffset; // NOTE: 4 per quad.
	u32 IndexArrayOffset; // NOTE: 6 per quad.
};

struct render_entry_meshes
{
	render_setup Setup;

	u32 MeshCount;
	u32 MeshArrayOffset;
};

enum camera_type
{
	Camera_Null,
	Camera_Orthogonal,
	Camera_Perspective,
};

INTROSPECT()
struct camera
{
	camera_type Type;
	union
	{
		struct
		{
			// NOTE: projection
			f32 HorizontalFOV;
			f32 AspectRatio;
		};

		struct
		{
			// NOTE: orthogonal
			f32 HalfWidth;
			f32 HalfHeight;
		};
	};

	f32 Near;
	f32 Far;

	v3 P;
	quaternion Rotation;

	mat4 Projection_;
	mat4 View_;
	mat4 InvView_;
};

enum render_group_flags
{
	Render_ClearColor = (1 << 0),
	Render_ClearDepth = (1 << 1),

	Render_Default = Render_ClearColor|Render_ClearDepth,
};
struct render_group
{
	struct assets *Assets;

	flag32(render_group_flags) Flags;

	render_setup LastSetup;
	render_entry_quads *CurrentQuadEntry;
	render_entry_meshes *CurrentMeshEntry;

	game_render_commands *RenderCommands;

	u32 MissingAssetsCount;

	renderer_texture WhiteTexture;
};

inline void
AddRenderOp(render_memory_op_queue *Queue, render_memory_op *Op)
{
	BeginTicketMutex(&Queue->Mutex);

	render_memory_op *NewOp = Queue->FirstFree;
	Assert(NewOp);
	Queue->FirstFree = NewOp->Next;

	*NewOp = *Op;
	Assert(!NewOp->Next);

	if(Queue->Last)
	{
		Queue->Last = Queue->Last->Next = NewOp;
	}
	else
	{
		Queue->Last = Queue->First = NewOp;
	}

	EndTicketMutex(&Queue->Mutex);
}

inline void
InitRenderMemoryOpQueue(render_memory_op_queue *RenderMemoryOpQueue, umm Size, void *Memory)
{
	umm TextureOpCount = (Size / sizeof(render_memory_op));
	render_memory_op *RenderOps = (render_memory_op *)Memory;
    for(umm Index = 0;
    	Index < (TextureOpCount - 1);
    	++Index)
    {
    	render_memory_op *Op = RenderOps + Index;
    	Op->Next = RenderOps + Index + 1;
    }

    RenderMemoryOpQueue->FirstFree = RenderOps;
}

internal renderer_texture
RendererTexture(u32 Index, u32 Width, u32 Height)
{
	renderer_texture Result = {};

	Result.Index = Index;
	Result.Width = (u16)Width;
	Result.Height = (u16)Height;

	Assert(Result.Index == Index);
	Assert(Result.Width == Width);
	Assert(Result.Height == Height);

	return Result;
}

internal renderer_mesh
RendererMesh(u32 Index, u32 VertexCount, u32 FaceCount)
{
	renderer_mesh Result = {};

	Result.Index = Index;
	Result.VertexCount = (u16)VertexCount;
	Result.FaceCount = (u16)FaceCount;

	Assert(Result.Index == Index);
	Assert(Result.VertexCount == VertexCount);
	Assert(Result.FaceCount == FaceCount);

	return Result;
}
