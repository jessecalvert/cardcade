/*@H
* File: cardcade_renderer.cpp
* Author: Jesse Calvert
* Created: February 1, 2018, 13:48
* Last modified: November 6, 2018, 13:11
*/

#include "cardcade_renderer.h"

inline void
MoveCamera(camera *Camera, v3 NewP)
{
	Camera->P = NewP;
}

inline void
RotateCamera(camera *Camera, quaternion NewRotation)
{
	Camera->Rotation = NormalizeIfNeeded(NewRotation);
}

inline void
ChangeCameraSettings(camera *Camera, f32 HalfWidth_HorizontalFOV, f32 HalfHeight_AspectRatio,
	f32 Near, f32 Far)
{
	switch(Camera->Type)
	{
		case Camera_Orthogonal:
		{
			Camera->HalfWidth = HalfWidth_HorizontalFOV;
			Camera->HalfHeight = HalfHeight_AspectRatio;
			Camera->Near = Near;
			Camera->Far = Far;
		} break;

		case Camera_Perspective:
		{
			Camera->HorizontalFOV = HalfWidth_HorizontalFOV;
			Camera->AspectRatio = HalfHeight_AspectRatio;
			Camera->Near = Near;
			Camera->Far = Far;
		} break;

		InvalidDefaultCase;
	}
}

internal camera
DefaultScreenCamera(v2i ScreenResolution)
{
	camera Result = {};
	Result.Type = Camera_Orthogonal;
	v2 ScreenDim = V2((f32)ScreenResolution.x, (f32)ScreenResolution.y);
	v3 ScreenCamP = V3(0.5f*ScreenDim, 5.0f);
	f32 ScreenCamNear = 0.0f;
	f32 ScreenCamFar = 10.0f;
	f32 ScreenCamWidthOverHeight = ScreenDim.x/ScreenDim.y;
	f32 ScreenCamHalfHeight = 0.5f*ScreenDim.y;
	f32 ScreenCamHalfWidth = ScreenCamHalfHeight*ScreenCamWidthOverHeight;
	ChangeCameraSettings(&Result, ScreenCamHalfWidth, ScreenCamHalfHeight,
		ScreenCamNear, ScreenCamFar);
	MoveCamera(&Result, ScreenCamP);
	return Result;
}

internal void
PushLight(render_group *Group, light Light)
{
	Assert(Group->RenderCommands->LightCount < MAX_LIGHTS);
	light *NewLight = Group->RenderCommands->Lights + Group->RenderCommands->LightCount++;
	*NewLight = Light;
}

internal void
PushAmbientLight(render_group *Group, f32 Coeff, v3 Color)
{
	light Light = {};
	Light.Type = LightType_Ambient;
	Light.Ambient.Coeff = Coeff;
	Light.Ambient.Color = Color;

	PushLight(Group, Light);
}

internal void
PushDirectionalLight(render_group *Group, v3 Direction, v3 Color, f32 Intensity, f32 ArcDegrees)
{
	light Light = {};
	Light.Type = LightType_Directional;
	Light.Directional.Color = Color;
	Light.Directional.Intensity = Intensity;
	Light.Directional.Direction = Direction;
	Light.Directional.ArcDegrees = ArcDegrees;

	PushLight(Group, Light);
}

internal void
PushPointLight(render_group *Group, v3 P, v3 Color, f32 Intensity, f32 Size, f32 Radius)
{
	light Light = {};
	Light.Type = LightType_Point;
	Light.Point.Color = Color;
	Light.Point.Intensity = Intensity;
	Light.Point.P = P;
	Light.Point.Size = Size;
	Light.Point.Radius = Radius;

	f32 TotalIntensity = Intensity * Maximum(Maximum(Color.r, Color.g), Color.b);
	f32 MinimumIntensity = 4.0f/256.0f;
	f32 InvMinimumIntensity = 1.0f/MinimumIntensity;

#if 0
	Light.Point.LinearAttenuation = 1.0f/Light.Point.Radius;
	Light.Point.QuadraticAttenuation = (InvMinimumIntensity*TotalIntensity - 2.0f)/Square(Light.Point.Radius);

#else
	Light.Point.LinearAttenuation = -TotalIntensity/Light.Point.Radius;
	Light.Point.QuadraticAttenuation = 0.0f;

#endif
	PushLight(Group, Light);
}

internal void
PushDecal(render_group *Group, v3 P, v3 Dim, quaternion Rotation, renderer_texture Texture)
{
	Assert(Group->RenderCommands->DecalCount < MAX_DECALS);
	decal *NewDecal = Group->RenderCommands->Decals + Group->RenderCommands->DecalCount++;
	NewDecal->Box.P = P;
	NewDecal->Box.Dim = Dim;
	NewDecal->Box.Rotation = Rotation;
	NewDecal->TextureHandle = Texture;
}

internal void
CameraSetMatrices(camera *Camera)
{
	Camera->Rotation = NormalizeOrNoRotation(Camera->Rotation);
	Camera->View_ = ViewMat4(Camera->P, Camera->Rotation);
	Camera->InvView_ = InvViewMat4(Camera->P, Camera->Rotation);

	switch(Camera->Type)
	{
		case Camera_Orthogonal:
		{
			f32 HalfWidth = Camera->HalfWidth;
			f32 HalfHeight = Camera->HalfHeight;
			Assert(HalfWidth && HalfHeight);

			Camera->Projection_ = OrthogonalProjectionMat4(HalfWidth, HalfHeight, Camera->Near, Camera->Far);
		} break;

		case Camera_Perspective:
		{
			Camera->Projection_ = PerspectiveProjectionMat4(Camera->HorizontalFOV, Camera->AspectRatio, Camera->Near, Camera->Far);
		} break;

		InvalidDefaultCase;
	}
}

internal void
PushSetup_(render_group *Group, render_setup *Setup)
{
	Group->LastSetup = *Setup;
	Group->CurrentQuadEntry = 0;
	Group->CurrentMeshEntry = 0;
}

internal void
SetCamera(render_group *Group, camera *Camera)
{
	Assert(Camera);

	CameraSetMatrices(Camera);

	render_setup NewSetup = Group->LastSetup;
	NewSetup.Projection = Camera->Projection_;
	NewSetup.View = Camera->View_;
	NewSetup.InvView = Camera->InvView_;
	NewSetup.FarPlane = Camera->Far;
	NewSetup.P = Camera->P;
	NewSetup.CameraRotation = Camera->Rotation;

	if(Camera->Type == Camera_Perspective)
	{
		f32 TanHalfHorizFOV = Tan(0.5f*Camera->HorizontalFOV);
		NewSetup.ViewRay = Camera->Far*V3(TanHalfHorizFOV,
										TanHalfHorizFOV/Camera->AspectRatio,
										-1.0f);
	}

	PushSetup_(Group, &NewSetup);
}

internal void
SetClipRect(render_group *Group, v2i ClipRectMin, v2i ClipRectMax)
{
	render_setup NewSetup = Group->LastSetup;
	NewSetup.ClipRectMin = ClipRectMin;
	NewSetup.ClipRectMax = ClipRectMax;
	PushSetup_(Group, &NewSetup);
}

internal void
SetRenderFlags(render_group *Group, flag32(render_setup_flags) NewFlags)
{
	render_setup NewSetup = Group->LastSetup;
	NewSetup.Flags = NewFlags;
	PushSetup_(Group, &NewSetup);
}

#define TemporaryRenderFlagChange__(Group, NewFlags, Number) temp_render_flag_setup TEMPORARY_RENDER_FLAGS##Number(Group, NewFlags)
#define TemporaryRenderFlagChange_(Group, NewFlags, Number) TemporaryRenderFlagChange__(Group, NewFlags, Number)
#define TemporaryRenderFlagChange(Group, NewFlags) TemporaryRenderFlagChange_(Group, NewFlags, __COUNTER__);

class temp_render_flag_setup
{
public:
	render_group *Group;
	u32 OldFlags;

	temp_render_flag_setup(render_group *GroupInit, u32 NewFlags)
	{
		Group = GroupInit;
		OldFlags = Group->LastSetup.Flags;
		SetRenderFlags(Group, NewFlags);
	}

	~temp_render_flag_setup()
	{
		SetRenderFlags(Group, OldFlags);
	}
};

#define TemporaryClipRectChange__(Group, ClipMin, ClipMax, Number) temp_clip_rect_setup TEMPORARY_CLIP_RECT##Number(Group, ClipMin, ClipMax)
#define TemporaryClipRectChange_(Group, ClipMin, ClipMax, Number) TemporaryClipRectChange__(Group, ClipMin, ClipMax, Number)
#define TemporaryClipRectChange(Group, ClipMin, ClipMax) TemporaryClipRectChange_(Group, ClipMin, ClipMax, __COUNTER__);

class temp_clip_rect_setup
{
public:
	render_group *Group;
	v2i OldClipMin;
	v2i OldClipMax;

	temp_clip_rect_setup(render_group *GroupInit, v2i ClipRectMin, v2i ClipRectMax)
	{
		Group = GroupInit;
		OldClipMin = Group->LastSetup.ClipRectMin;
		OldClipMax = Group->LastSetup.ClipRectMax;
		SetClipRect(Group, ClipRectMin, ClipRectMax);
	}

	~temp_clip_rect_setup()
	{
		SetClipRect(Group, OldClipMin, OldClipMax);
	}
};

#define PushRenderEntry(Group, type) (type *)PushRenderEntry_((Group), sizeof(type), RenderCommandType_##type)
internal u8 *
PushRenderEntry_(render_group *Group,
	u32 EntrySize, render_command_type Type)
{
	game_render_commands *RenderCommands = Group->RenderCommands;
	u32 TotalSize = EntrySize + sizeof(render_entry_header);
	Assert(TotalSize < (RenderCommands->MaxSize - RenderCommands->Size));

	render_entry_header *Header = (render_entry_header *)(RenderCommands->Buffer + RenderCommands->Size);
	RenderCommands->Size += TotalSize;

	Header->Type = Type;

	u8 *Result = (u8 *)(Header + 1);
	return Result;
}

internal void
PushFullClear_(render_group *Group, v4 Color)
{
	render_entry_full_clear *FullClear = PushRenderEntry(Group, render_entry_full_clear);
	FullClear->Color = Color;
}

internal void
PushDepthClear_(render_group *Group)
{
	PushRenderEntry_(Group, 0, RenderCommandType_render_entry_clear_depth);
}

internal render_entry_quads *
GetQuadEntry(render_group *Group)
{
	render_entry_quads *Result = Group->CurrentQuadEntry;
	if(!Result)
	{
		Result = Group->CurrentQuadEntry = PushRenderEntry(Group, render_entry_quads);
		Result->VertexArrayOffset = Group->RenderCommands->VertexCount;
		Result->IndexArrayOffset = Group->RenderCommands->IndexCount;
		Result->QuadCount = 0;
		Result->Setup = Group->LastSetup;
	}

	return Result;
}

internal render_entry_meshes *
GetMeshEntry(render_group *Group)
{
	render_entry_meshes *Result = Group->CurrentMeshEntry;
	if(!Result)
	{
		Result = Group->CurrentMeshEntry = PushRenderEntry(Group, render_entry_meshes);
		Result->MeshArrayOffset = Group->RenderCommands->MeshCount;
		Result->MeshCount = 0;
		Result->Setup = Group->LastSetup;
	}

	return Result;
}

inline u32
PackV4ToU32(v4 V)
{
	V.r = Clamp01(V.r);
	V.g = Clamp01(V.g);
	V.b = Clamp01(V.b);
	V.a = Clamp01(V.a);

	u32 R = (u32)(255.0f*V.r);
	u32 G = (u32)(255.0f*V.g);
	u32 B = (u32)(255.0f*V.b);
	u32 A = (u32)(255.0f*V.a);

	u32 Result = ((A << 24) |
		          (B << 16) |
		          (G << 8) |
		          (R << 0));
	return Result;
}

internal void
BeginDepthPeels_(render_group *Group)
{
	PushRenderEntry_(Group, 0, RenderCommandType_render_entry_begin_depth_peels);
}

internal void
EndDepthPeels_(render_group *Group)
{
	PushRenderEntry_(Group, 0, RenderCommandType_render_entry_end_depth_peels);
}

internal void
PushTarget_(render_group *Group, render_target_type Target)
{
	render_entry_set_target *SetTarget = PushRenderEntry(Group, render_entry_set_target);
	SetTarget->Target = Target;
}

internal render_group
BeginRender(assets *Assets, game_render_commands *RenderCommands,
	render_target_type Target, v4 ClearColor = {}, flag32(render_group_flags) Flags = Render_Default)
{
	Assert(!RenderCommands->InUse);

	render_group Group = {};
	Group.RenderCommands = RenderCommands;
	Group.Assets = Assets;
	RenderCommands->InUse = true;

	v2i Resolution = RenderCommands->Settings.Resolution;
	Group.LastSetup.Target = Target;
	Group.LastSetup.ClipRectMax = Resolution;
	camera DefaultCamera = DefaultScreenCamera(Resolution);
	SetCamera(&Group, &DefaultCamera);

	Group.WhiteTexture = RenderCommands->WhiteTexture;

	if(Group.LastSetup.Target == Target_Scene)
	{
		BeginDepthPeels_(&Group);
	}

	PushTarget_(&Group, Group.LastSetup.Target);

	if(Flags & Render_ClearColor)
	{
		Assert(Flags & Render_ClearDepth);
		PushFullClear_(&Group, ClearColor);
	}
	else if(Flags & Render_ClearDepth)
	{
		PushDepthClear_(&Group);
	}

	return Group;
}

internal void
EndRender(render_group *Group)
{
	if(Group->LastSetup.Target == Target_Scene)
	{
		EndDepthPeels_(Group);
	}

	Group->RenderCommands->InUse = false;
	Group->RenderCommands = 0;
	Group->Assets = 0;
}

internal void
PushMesh(render_group *Group, v3 P, quaternion Rotation, v3 Scale,
	renderer_mesh MeshHandle, renderer_texture TextureHandle,
	surface_material Material, b32 Wireframe = false)
{
	game_render_commands *RenderCommands = Group->RenderCommands;
	render_entry_meshes *Entry = GetMeshEntry(Group);
	u32 MeshIndex = Entry->MeshArrayOffset + Entry->MeshCount;
	++Entry->MeshCount;
	++RenderCommands->MeshCount;
	render_command_mesh_data *MeshEntry = RenderCommands->MeshArray + MeshIndex;
	Assert(RenderCommands->MeshCount < RenderCommands->MaxMeshCount);

	MeshEntry->Model = TranslationMat4(P)*RotationMat4(Rotation)*ScaleMat4(Scale);
	MeshEntry->NormalMatrix = Transpose(Inverse(Mat3(MeshEntry->Model.C[0].xyz,
													 MeshEntry->Model.C[1].xyz,
													 MeshEntry->Model.C[2].xyz)));
	MeshEntry->Material = Material;
	MeshEntry->TextureIndex = TextureHandle.Index;
	MeshEntry->MeshIndex = MeshHandle.Index;
}

internal void
PushQuad_(render_group *Group,
	v3 Vert0, v3 Vert1, v3 Vert2, v3 Vert3,
	v2 TexMin, v2 TexMax,
	renderer_texture TextureHandle,
	b32 UseNormals, f32 Roughness, f32 Metalness,
	v4 Color = V4(1,1,1,1))
{
	game_render_commands *RenderCommands = Group->RenderCommands;
	render_entry_quads *Entry = GetQuadEntry(Group);

	u32 VertexIndex = Entry->VertexArrayOffset + 4*Entry->QuadCount;
	u32 IndexIndex = Entry->IndexArrayOffset + 6*Entry->QuadCount;

	vertex_format *Verts = RenderCommands->VertexArray + VertexIndex;
	u32 *Indexes = RenderCommands->IndexArray + IndexIndex;

	++Entry->QuadCount;
	RenderCommands->VertexCount += 4;
	RenderCommands->IndexCount += 6;
	Assert(RenderCommands->VertexCount < RenderCommands->MaxVertexCount);
	Assert(RenderCommands->IndexCount < RenderCommands->MaxIndexCount);

	Verts[0].Position = Vert0;
	Verts[1].Position = Vert1;
	Verts[2].Position = Vert2;
	Verts[3].Position = Vert3;

	if(UseNormals)
	{
		v3 Normal = NOZ(Cross(Vert2 - Vert1, Vert0 - Vert1));
		Verts[0].Normal = Normal;
		Verts[1].Normal = Normal;
		Verts[2].Normal = Normal;
		Verts[3].Normal = Normal;
	}

	v2 TexScale = V2((f32)TextureHandle.Width/(f32)MAX_TEXTURE_DIM,
					 (f32)TextureHandle.Height/(f32)MAX_TEXTURE_DIM);
	v2 TexDim = TexMax - TexMin;
	Verts[0].TexCoords = Hadamard(TexScale, TexMin);
	Verts[1].TexCoords = Hadamard(TexScale, TexMin + V2(TexDim.x, 0.0f));
	Verts[2].TexCoords = Hadamard(TexScale, TexMax);
	Verts[3].TexCoords = Hadamard(TexScale, TexMin + V2(0.0f, TexDim.y));

	u32 ColorPacked = PackV4ToU32(Color);
	Verts[0].Color = ColorPacked;
	Verts[1].Color = ColorPacked;
	Verts[2].Color = ColorPacked;
	Verts[3].Color = ColorPacked;

	Verts[0].TextureIndex = TextureHandle.Index;
	Verts[1].TextureIndex = TextureHandle.Index;
	Verts[2].TextureIndex = TextureHandle.Index;
	Verts[3].TextureIndex = TextureHandle.Index;

	v2 Roughness_Metalness = V2(Roughness, Metalness);
	Verts[0].Roughness_Metalness = Roughness_Metalness;
	Verts[1].Roughness_Metalness = Roughness_Metalness;
	Verts[2].Roughness_Metalness = Roughness_Metalness;
	Verts[3].Roughness_Metalness = Roughness_Metalness;

	Indexes[0] = VertexIndex + 0;
	Indexes[1] = VertexIndex + 1;
	Indexes[2] = VertexIndex + 3;
	Indexes[3] = VertexIndex + 1;
	Indexes[4] = VertexIndex + 2;
	Indexes[5] = VertexIndex + 3;
}

internal void
PushQuad_(render_group *Group, v2 Min, v2 Max,
	v2 TexMin, v2 TexMax, mat2 RotationMatrix, renderer_texture TextureHandle,
	f32 Roughness, f32 Metalness, v4 Color = V4(1,1,1,1))
{
	v2 Dim = Max - Min;
	v2 P = Min + 0.5f*Dim;
	v3 Vert0 = V3(RotationMatrix*(Min - P) + P, 0.0f);
	v3 Vert1 = V3(RotationMatrix*(Min + V2(Dim.x, 0.0f) - P) + P, 0.0f);
	v3 Vert2 = V3(RotationMatrix*(Max - P) + P, 0.0f);
	v3 Vert3 = V3(RotationMatrix*(Min + V2(0.0f, Dim.y) - P) + P, 0.0f);

	PushQuad_(Group, Vert0, Vert1, Vert2, Vert3, TexMin, TexMax, TextureHandle, false, Roughness, Metalness, Color);
}

internal void
PushQuad_(render_group *Group, v2 Min, v2 Max,
	v2 TexMin, v2 TexMax, f32 Rotation, renderer_texture TextureHandle,
	f32 Roughness, f32 Metalness, v4 Color = V4(1,1,1,1))
{
	v2 Dim = Max - Min;
	v2 P = Min + 0.5f*Dim;
	if(Rotation)
	{
		mat2 RotationMatrix = RotationMat2(Rotation);
		v3 Vert0 = V3(RotationMatrix*(Min - P) + P, 0.0f);
		v3 Vert1 = V3(RotationMatrix*(Min + V2(Dim.x, 0.0f) - P) + P, 0.0f);
		v3 Vert2 = V3(RotationMatrix*(Max - P) + P, 0.0f);
		v3 Vert3 = V3(RotationMatrix*(Min + V2(0.0f, Dim.y) - P) + P, 0.0f);

		PushQuad_(Group, Vert0, Vert1, Vert2, Vert3, TexMin, TexMax, TextureHandle, false, Roughness, Metalness, Color);
	}
	else
	{
		v3 Vert0 = V3(Min, 0.0f);
		v3 Vert1 = V3(Min + V2(Dim.x, 0.0f), 0.0f);
		v3 Vert2 = V3(Max, 0.0f);
		v3 Vert3 = V3(Min + V2(0.0f, Dim.y), 0.0f);

		PushQuad_(Group, Vert0, Vert1, Vert2, Vert3, TexMin, TexMax, TextureHandle, false, Roughness, Metalness, Color);
	}
}

internal void
PushQuad(render_group *Group, v2 P, v2 Dim, mat2 Rotation, v4 Color)
{
	v2 Min = P - 0.5f*Dim;
	v2 Max = P + 0.5f*Dim;
	renderer_texture WhiteTexture = Group->WhiteTexture;
	PushQuad_(Group, Min, Max, V2(0,0), V2(1,1), Rotation, WhiteTexture, 1.0f, 0.0f, Color);
}

internal void
PushQuad(render_group *Group, v2 Min, v2 Max, v4 Color = V4(1,1,1,1))
{
	renderer_texture WhiteTexture = Group->WhiteTexture;
	PushQuad_(Group, Min, Max, V2(0,0), V2(1,1), 0.0f, WhiteTexture, 1.0f, 0.0f, Color);
}

internal void
PushRectangleOutline(render_group *Group, v2 Min, v2 Max, v4 Color, r32 Width = 0.1f)
{
	PushQuad(Group, V2(Min.x - Width, Min.y - Width), V2(Max.x + Width, Min.y), Color);
	PushQuad(Group, V2(Min.x - Width, Min.y), V2(Min.x, Max.y + Width), Color);
	PushQuad(Group, V2(Min.x, Max.y), V2(Max.x + Width, Max.y + Width), Color);
	PushQuad(Group, V2(Max.x, Min.y), V2(Max.x + Width, Max.y), Color);
}

internal void
PushCube(render_group *Group, v3 P,
	v3 HalfXAxis, v3 HalfYAxis, v3 HalfZAxis,
	f32 Roughness, f32 Metalness, v4 Color)
{
	v3 P0 = P + HalfXAxis - HalfYAxis + HalfZAxis;
	v3 P1 = P + HalfXAxis - HalfYAxis - HalfZAxis;
	v3 P2 = P + HalfXAxis + HalfYAxis - HalfZAxis;
	v3 P3 = P + HalfXAxis + HalfYAxis + HalfZAxis;
	v3 P4 = P - HalfXAxis - HalfYAxis - HalfZAxis;
	v3 P5 = P - HalfXAxis - HalfYAxis + HalfZAxis;
	v3 P6 = P - HalfXAxis + HalfYAxis + HalfZAxis;
	v3 P7 = P - HalfXAxis + HalfYAxis - HalfZAxis;

	// NOTE: +X
	PushQuad_(Group,
		P0, P1, P2, P3,
		V2(0,0), V2(1,1), Group->WhiteTexture, true, Roughness, Metalness, Color);

	// NOTE: -X
	PushQuad_(Group,
		P4, P5, P6, P7,
		V2(0,0), V2(1,1), Group->WhiteTexture, true, Roughness, Metalness, Color);

	// NOTE: +Y
	PushQuad_(Group,
		P6, P3, P2, P7,
		V2(0,0), V2(1,1), Group->WhiteTexture, true, Roughness, Metalness, Color);

	// NOTE: -Y
	PushQuad_(Group,
		P4, P1, P0, P5,
		V2(0,0), V2(1,1), Group->WhiteTexture, true, Roughness, Metalness, Color);

	// NOTE: +Z
	PushQuad_(Group,
		P5, P0, P3, P6,
		V2(0,0), V2(1,1), Group->WhiteTexture, true, Roughness, Metalness, Color);

	// NOTE: -Z
	PushQuad_(Group,
		P1, P4, P7, P2,
		V2(0,0), V2(1,1), Group->WhiteTexture, true, Roughness, Metalness, Color);
}

internal void
PushCube(render_group *Group, v3 P, v3 Dim, quaternion Rotation, f32 Roughness, f32 Metalness, v4 Color)
{
	v3 HalfDim = 0.5f*Dim;
	v3 HalfXAxis = RotateBy(V3(HalfDim.x, 0, 0), Rotation);
	v3 HalfYAxis = RotateBy(V3(0, HalfDim.y, 0), Rotation);
	v3 HalfZAxis = RotateBy(V3(0, 0, HalfDim.z), Rotation);
	PushCube(Group, P, HalfXAxis, HalfYAxis, HalfZAxis, Roughness, Metalness, Color);
}

internal void
PushCube(render_group *Group, v3 P, v3 Dim, f32 Roughness, f32 Metalness, v4 Color)
{
	v3 HalfDim = 0.5f*Dim;
	v3 HalfXAxis = V3(HalfDim.x, 0, 0);
	v3 HalfYAxis = V3(0, HalfDim.y, 0);
	v3 HalfZAxis = V3(0, 0, HalfDim.z);
	PushCube(Group, P, HalfXAxis, HalfYAxis, HalfZAxis, Roughness, Metalness, Color);
}

internal void
PushTexture(render_group *Group, v3 P, quaternion Rotation, v2 Scale,
	renderer_texture TextureHandle,
	f32 Roughness, f32 Metalness, v4 Color = V4(1,1,1,1))
{
	v3 HalfXAxis = 0.5f*RotateBy(V3(Scale.x, 0, 0), Rotation);
	v3 HalfYAxis = 0.5f*RotateBy(V3(0, Scale.y, 0), Rotation);

	v3 Verts[] =
	{
		P + -HalfXAxis + -HalfYAxis,
		P + HalfXAxis + -HalfYAxis,
		P + HalfXAxis + HalfYAxis,
		P + -HalfXAxis + HalfYAxis,
	};

	PushQuad_(Group, Verts[0], Verts[1], Verts[2], Verts[3], V2(0,0), V2(1,1),
		TextureHandle, true, Roughness, Metalness, Color);
}

internal void
PushTexture(render_group *Group, v2 P, v2 XAxis, v2 YAxis, renderer_texture TextureHandle)
{
	v2 HalfXAxis = 0.5f * XAxis;
	v2 HalfYAxis = 0.5f * YAxis;
	PushQuad_(Group,
		V3(P - HalfXAxis - HalfYAxis, 0),
		V3(P + HalfXAxis - HalfYAxis, 0),
		V3(P + HalfXAxis + HalfYAxis, 0),
		V3(P - HalfXAxis + HalfYAxis, 0),
		V2(0,0), V2(1,1), TextureHandle, true, 1.0f, 0.0f);
}

internal void
PushUprightTexture(render_group *Group, v3 P, v2 Scale,
	renderer_texture TextureHandle, f32 Roughness, f32 Metalness, v4 Color = V4(1,1,1,1))
{
	quaternion CameraRotation = Group->LastSetup.CameraRotation;
	PushTexture(Group, P, CameraRotation, Scale, TextureHandle, Roughness, Metalness, Color);
}

internal void
PushRectangle3Outline(render_group *Group, v3 Min, v3 Max, v4 Color, r32 Width = 0.1f)
{
	v3 Dim = Max - Min;
	v3 P = Min + 0.5f*Dim;
	PushCube(Group, P, Dim, Q(1,0,0,0), 1.0f, 0.0f, Color);
}

internal void
PushPoint(render_group *Group, v2 P, v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
	v2 HalfPointDim = 0.5f*V2(0.1f, 0.1f);
	PushQuad(Group, (P - HalfPointDim), (P + HalfPointDim), Color);
}

internal void
PushPoint(render_group *Group, v3 P, v3 Color = V3(1.0f, 1.0f, 1.0f))
{
	v2 Dim = V2(0.1f, 0.1f);
	PushUprightTexture(Group, P, Dim, Group->WhiteTexture, 1.0f, 0.0f);
}

internal void
PushLine(render_group *Group, v2 A, v2 B, f32 Width, v4 Color = V4(1.0f, 1.0f, 0.0f, 1.0f))
{
	v2 Line = B - A;
	r32 Distance = Length(Line);
	v2 Dim = V2(Distance, Width);
	v2 P = A + 0.5f*(Line);
	f32 Rotation = ATan2(Line.y, Line.x);
	PushQuad(Group, P, Dim, RotationMat2(Rotation), Color);
}

internal void
PushLine(render_group *Group, v3 A, v3 B, f32 Width, v4 Color = V4(1.0f, 1.0f, 0.0f, 1.0f))
{
	v3 Line = B - A;

	r32 Distance = Length(Line);
	v3 Dim = V3(Distance, Width, Width);
	v3 P = A + 0.5f*(Line);
	v3 LineUnit = NOZ(Line);
	v3 DefaultDirection = V3(1,0,0);
	v3 RotationAxis = NormalizeOr(Cross(DefaultDirection, LineUnit), DefaultDirection);
	f32 RotationAngle = Arccos(Inner(DefaultDirection, LineUnit));
	quaternion Rotation = RotationQuaternion(RotationAxis, RotationAngle);

	PushCube(Group, P, Dim, Rotation, 1.0f, 0.0f, Color);
}
