/*@H
* File: generated.h
* Author: simple_meta.exe
* Created: November 27, 2017, 11:00
* Last modified: November 27, 2017, 11:00
*/

#pragma once

#define MAX_CIRCULAR_REFERENCE_DEPTH 2

#include "cardcade_debug.h"
#include "cardcade_editor.h"
#include "cardcade_title_screen.h"
#include "cardcade_entity_manager.h"
#include "cardcade_battle_mode.h"
#include "cardcade_ability.h"
#include "cardcade_ui.h"
#include "cardcade_particles.h"
#include "cardcade_animation.h"
#include "cardcade_convex_hull.h"
#include "cardcade_rigid_body.h"
#include "cardcade_gjk.h"
#include "cardcade_aabb.h"
#include "cardcade_random.h"
#include "cardcade_sort.h"
#include "cardcade_assets.h"
#include "cardcade_file_formats.h"
#include "cardcade_config.h"
#include "cardcade_renderer.h"
#include "cardcade_stream.h"
#include "cardcade_memory.h"
#include "cardcade_math.h"
#include "cardcade_simd.h"
#include "cardcade_intrinsics.h"
#include "cardcade_debug_interface.h"
#include "cardcade_platform.h"
#include "cardcade_shared.h"
#include "cardcade_types.h"
#include "cardcade.h"

global char * Names_shader_type[] =
{
    "Shader_Null",
    "Shader_Header",
    "Shader_Scene_Quads",
    "Shader_Scene_Quads_SDF",
    "Shader_Scene_Meshes",
    "Shader_ResolveDepthPeel",
    "Shader_CompositeDepthPeel",
    "Shader_Overlay",
    "Shader_Overlay_SDF",
    "Shader_Overbright",
    "Shader_Blur",
    "Shader_Finalize",
    "Shader_DebugView",
    "Shader_Count",
};

global char * Names_entity_visible_piece_type[] =
{
    "PieceType_Cube",
    "PieceType_Bitmap",
    "PieceType_Bitmap_Upright",
    "PieceType_Mesh",
    "PieceType_EntityInfo",
    "PieceType_Count",
};

global char * Names_entity_type[] =
{
    "Entity_None",
    "Entity_Ground",
    "Entity_Water",
    "Entity_Forest",
    "Entity_Mountain",
    "Entity_Warrior",
    "Entity_Mercenaries",
    "Entity_Bank",
    "Entity_RiskInvestment",
    "Entity_GoldMine",
    "Entity_PowerPlant",
    "Entity_Headquarters",
    "Entity_Card",
    "Entity_Count",
};

global char * Names_ability_name[] =
{
    "Ability_Move",
    "Ability_Attack",
    "Ability_GenerateCharge",
    "Ability_BuildHeadquarters",
    "Ability_DrawCard",
    "Ability_GainGold",
    "Ability_GatherResources",
    "Ability_HQPillaged",
    "Ability_BuildBank",
    "Ability_BankActive",
    "Ability_BankPillaged",
    "Ability_BuildRiskInvestment",
    "Ability_RiskInvestmentReturn",
    "Ability_RiskInvestmentPillaged",
    "Ability_BuildGoldMine",
    "Ability_GoldMine",
    "Ability_GoldMinePillaged",
    "Ability_BuildPowerPlant",
    "Ability_GeneratePower",
    "Ability_PowerPlantPillaged",
    "Ability_SummonWarrior",
    "Ability_Fireball",
    "Ability_SummonMercenaries",
    "Ability_HireMercenaries",
    "Ability_HedgeFund",
    "Ability_Buyout",
    "Ability_Bribe",
    "Ability_Fortify",
    "Ability_Aura",
    "Ability_Counter",
    "Ability_GenerateFood",
    "Ability_CreateResources",
    "Ability_Count",
};

global char * Names_asset_name[] =
{
    "Asset_None",
    "Asset_Circle",
    "Asset_Smile",
    "Asset_Arrow",
    "Asset_GoldIcon",
    "Asset_HeartIcon",
    "Asset_ManaIcon",
    "Asset_SwordIcon",
    "Asset_ClockIcon",
    "Asset_LiberationMonoFont",
    "Asset_TimesFont",
    "Asset_RoundedBox",
    "Asset_Sphere",
    "Asset_Capsule",
    "Asset_EntityInfo",
    "Asset_Count",
};

global char * Names_asset_type[] =
{
    "AssetType_None",
    "AssetType_Bitmap",
    "AssetType_Font",
    "AssetType_Mesh",
    "AssetType_EntityInfo",
    "AssetType_Count",
};

global char * Names_shape_type[] =
{
    "Shape_None",
    "Shape_Sphere",
    "Shape_Capsule",
    "Shape_ConvexHull",
    "Shape_Count",
};

global char * Names_ability_type[] =
{
    "AbilityType_Passive",
    "AbilityType_Reaction",
    "AbilityType_Activated",
};

global char * Names_reaction_trigger[] =
{
    "Reaction_TurnStart",
    "Reaction_TurnEnd",
    "Reaction_OnDestroy",
    "Reaction_OnChargeChange",
};

global char * Names_player_name[] =
{
    "Player_None",
    "Player_Neutral",
    "Player_1",
    "Player_2",
    "Player_Count",
};

global char * Names_game_phase[] =
{
    "Phase_None",
    "Phase_GameBegin",
    "Phase_PlaceHQ",
    "Phase_HQEnd",
    "Phase_TurnStart",
    "Phase_PlayerActions",
    "Phase_TurnEnd",
    "Phase_GameOver",
};

inline void RecordDebugValue_(render_settings *ValuePtr, char *GUID, u32 Depth=0);
inline void RecordDebugValue_(camera *ValuePtr, char *GUID, u32 Depth=0);
inline void RecordDebugValue_(shape *ValuePtr, char *GUID, u32 Depth=0);
inline void RecordDebugValue_(collider *ValuePtr, char *GUID, u32 Depth=0);
inline void RecordDebugValue_(rigid_body *ValuePtr, char *GUID, u32 Depth=0);
inline void RecordDebugValue_(convex_hull *ValuePtr, char *GUID, u32 Depth=0);
inline void RecordDebugValue_(entity *ValuePtr, char *GUID, u32 Depth=0);

inline void RecordDebugValue_(shader_type *ValuePtr, char *GUID)
{
	switch(*ValuePtr)
	{
        case 0: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_Null"), Event_enum);} break;
        case 1: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_Header"), Event_enum);} break;
        case 2: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_Scene_Quads"), Event_enum);} break;
        case 3: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_Scene_Quads_SDF"), Event_enum);} break;
        case 4: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_Scene_Meshes"), Event_enum);} break;
        case 5: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_ResolveDepthPeel"), Event_enum);} break;
        case 6: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_CompositeDepthPeel"), Event_enum);} break;
        case 7: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_Overlay"), Event_enum);} break;
        case 8: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_Overlay_SDF"), Event_enum);} break;
        case 9: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_Overbright"), Event_enum);} break;
        case 10: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_Blur"), Event_enum);} break;
        case 11: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_Finalize"), Event_enum);} break;
        case 12: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_DebugView"), Event_enum);} break;
        case 13: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shader_Count"), Event_enum);} break;
	}
}

inline void RecordDebugValue_(entity_visible_piece_type *ValuePtr, char *GUID)
{
	switch(*ValuePtr)
	{
        case 0: {RECORD_DEBUG_EVENT(DEBUG_NAME("PieceType_Cube"), Event_enum);} break;
        case 1: {RECORD_DEBUG_EVENT(DEBUG_NAME("PieceType_Bitmap"), Event_enum);} break;
        case 2: {RECORD_DEBUG_EVENT(DEBUG_NAME("PieceType_Bitmap_Upright"), Event_enum);} break;
        case 3: {RECORD_DEBUG_EVENT(DEBUG_NAME("PieceType_Mesh"), Event_enum);} break;
        case 4: {RECORD_DEBUG_EVENT(DEBUG_NAME("PieceType_EntityInfo"), Event_enum);} break;
        case 5: {RECORD_DEBUG_EVENT(DEBUG_NAME("PieceType_Count"), Event_enum);} break;
	}
}

inline void RecordDebugValue_(entity_type *ValuePtr, char *GUID)
{
	switch(*ValuePtr)
	{
        case 0: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_None"), Event_enum);} break;
        case 1: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_Ground"), Event_enum);} break;
        case 2: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_Water"), Event_enum);} break;
        case 3: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_Forest"), Event_enum);} break;
        case 4: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_Mountain"), Event_enum);} break;
        case 5: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_Warrior"), Event_enum);} break;
        case 6: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_Mercenaries"), Event_enum);} break;
        case 7: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_Bank"), Event_enum);} break;
        case 8: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_RiskInvestment"), Event_enum);} break;
        case 9: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_GoldMine"), Event_enum);} break;
        case 10: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_PowerPlant"), Event_enum);} break;
        case 11: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_Headquarters"), Event_enum);} break;
        case 12: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_Card"), Event_enum);} break;
        case 13: {RECORD_DEBUG_EVENT(DEBUG_NAME("Entity_Count"), Event_enum);} break;
	}
}

inline void RecordDebugValue_(ability_name *ValuePtr, char *GUID)
{
	switch(*ValuePtr)
	{
        case 0: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_Move"), Event_enum);} break;
        case 1: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_Attack"), Event_enum);} break;
        case 2: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_GenerateCharge"), Event_enum);} break;
        case 3: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_BuildHeadquarters"), Event_enum);} break;
        case 4: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_DrawCard"), Event_enum);} break;
        case 5: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_GainGold"), Event_enum);} break;
        case 6: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_GatherResources"), Event_enum);} break;
        case 7: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_HQPillaged"), Event_enum);} break;
        case 8: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_BuildBank"), Event_enum);} break;
        case 9: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_BankActive"), Event_enum);} break;
        case 10: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_BankPillaged"), Event_enum);} break;
        case 11: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_BuildRiskInvestment"), Event_enum);} break;
        case 12: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_RiskInvestmentReturn"), Event_enum);} break;
        case 13: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_RiskInvestmentPillaged"), Event_enum);} break;
        case 14: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_BuildGoldMine"), Event_enum);} break;
        case 15: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_GoldMine"), Event_enum);} break;
        case 16: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_GoldMinePillaged"), Event_enum);} break;
        case 17: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_BuildPowerPlant"), Event_enum);} break;
        case 18: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_GeneratePower"), Event_enum);} break;
        case 19: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_PowerPlantPillaged"), Event_enum);} break;
        case 20: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_SummonWarrior"), Event_enum);} break;
        case 21: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_Fireball"), Event_enum);} break;
        case 22: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_SummonMercenaries"), Event_enum);} break;
        case 23: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_HireMercenaries"), Event_enum);} break;
        case 24: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_HedgeFund"), Event_enum);} break;
        case 25: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_Buyout"), Event_enum);} break;
        case 26: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_Bribe"), Event_enum);} break;
        case 27: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_Fortify"), Event_enum);} break;
        case 28: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_Aura"), Event_enum);} break;
        case 29: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_Counter"), Event_enum);} break;
        case 30: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_GenerateFood"), Event_enum);} break;
        case 31: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_CreateResources"), Event_enum);} break;
        case 32: {RECORD_DEBUG_EVENT(DEBUG_NAME("Ability_Count"), Event_enum);} break;
	}
}

inline void RecordDebugValue_(asset_name *ValuePtr, char *GUID)
{
	switch(*ValuePtr)
	{
        case 0: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_None"), Event_enum);} break;
        case 1: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_Circle"), Event_enum);} break;
        case 2: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_Smile"), Event_enum);} break;
        case 3: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_Arrow"), Event_enum);} break;
        case 4: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_GoldIcon"), Event_enum);} break;
        case 5: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_HeartIcon"), Event_enum);} break;
        case 6: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_ManaIcon"), Event_enum);} break;
        case 7: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_SwordIcon"), Event_enum);} break;
        case 8: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_ClockIcon"), Event_enum);} break;
        case 9: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_LiberationMonoFont"), Event_enum);} break;
        case 10: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_TimesFont"), Event_enum);} break;
        case 11: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_RoundedBox"), Event_enum);} break;
        case 12: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_Sphere"), Event_enum);} break;
        case 13: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_Capsule"), Event_enum);} break;
        case 14: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_EntityInfo"), Event_enum);} break;
        case 15: {RECORD_DEBUG_EVENT(DEBUG_NAME("Asset_Count"), Event_enum);} break;
	}
}

inline void RecordDebugValue_(asset_type *ValuePtr, char *GUID)
{
	switch(*ValuePtr)
	{
        case 0: {RECORD_DEBUG_EVENT(DEBUG_NAME("AssetType_None"), Event_enum);} break;
        case 1: {RECORD_DEBUG_EVENT(DEBUG_NAME("AssetType_Bitmap"), Event_enum);} break;
        case 2: {RECORD_DEBUG_EVENT(DEBUG_NAME("AssetType_Font"), Event_enum);} break;
        case 3: {RECORD_DEBUG_EVENT(DEBUG_NAME("AssetType_Mesh"), Event_enum);} break;
        case 4: {RECORD_DEBUG_EVENT(DEBUG_NAME("AssetType_EntityInfo"), Event_enum);} break;
        case 5: {RECORD_DEBUG_EVENT(DEBUG_NAME("AssetType_Count"), Event_enum);} break;
	}
}

inline void RecordDebugValue_(shape_type *ValuePtr, char *GUID)
{
	switch(*ValuePtr)
	{
        case 0: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shape_None"), Event_enum);} break;
        case 1: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shape_Sphere"), Event_enum);} break;
        case 2: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shape_Capsule"), Event_enum);} break;
        case 3: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shape_ConvexHull"), Event_enum);} break;
        case 4: {RECORD_DEBUG_EVENT(DEBUG_NAME("Shape_Count"), Event_enum);} break;
	}
}

inline void RecordDebugValue_(ability_type *ValuePtr, char *GUID)
{
	switch(*ValuePtr)
	{
        case 0: {RECORD_DEBUG_EVENT(DEBUG_NAME("AbilityType_Passive"), Event_enum);} break;
        case 1: {RECORD_DEBUG_EVENT(DEBUG_NAME("AbilityType_Reaction"), Event_enum);} break;
        case 2: {RECORD_DEBUG_EVENT(DEBUG_NAME("AbilityType_Activated"), Event_enum);} break;
	}
}

inline void RecordDebugValue_(reaction_trigger *ValuePtr, char *GUID)
{
	switch(*ValuePtr)
	{
        case 0: {RECORD_DEBUG_EVENT(DEBUG_NAME("Reaction_TurnStart"), Event_enum);} break;
        case 1: {RECORD_DEBUG_EVENT(DEBUG_NAME("Reaction_TurnEnd"), Event_enum);} break;
        case 2: {RECORD_DEBUG_EVENT(DEBUG_NAME("Reaction_OnDestroy"), Event_enum);} break;
        case 3: {RECORD_DEBUG_EVENT(DEBUG_NAME("Reaction_OnChargeChange"), Event_enum);} break;
	}
}

inline void RecordDebugValue_(player_name *ValuePtr, char *GUID)
{
	switch(*ValuePtr)
	{
        case 0: {RECORD_DEBUG_EVENT(DEBUG_NAME("Player_None"), Event_enum);} break;
        case 1: {RECORD_DEBUG_EVENT(DEBUG_NAME("Player_Neutral"), Event_enum);} break;
        case 2: {RECORD_DEBUG_EVENT(DEBUG_NAME("Player_1"), Event_enum);} break;
        case 3: {RECORD_DEBUG_EVENT(DEBUG_NAME("Player_2"), Event_enum);} break;
        case 4: {RECORD_DEBUG_EVENT(DEBUG_NAME("Player_Count"), Event_enum);} break;
	}
}

inline void RecordDebugValue_(game_phase *ValuePtr, char *GUID)
{
	switch(*ValuePtr)
	{
        case 0: {RECORD_DEBUG_EVENT(DEBUG_NAME("Phase_None"), Event_enum);} break;
        case 1: {RECORD_DEBUG_EVENT(DEBUG_NAME("Phase_GameBegin"), Event_enum);} break;
        case 2: {RECORD_DEBUG_EVENT(DEBUG_NAME("Phase_PlaceHQ"), Event_enum);} break;
        case 3: {RECORD_DEBUG_EVENT(DEBUG_NAME("Phase_HQEnd"), Event_enum);} break;
        case 4: {RECORD_DEBUG_EVENT(DEBUG_NAME("Phase_TurnStart"), Event_enum);} break;
        case 5: {RECORD_DEBUG_EVENT(DEBUG_NAME("Phase_PlayerActions"), Event_enum);} break;
        case 6: {RECORD_DEBUG_EVENT(DEBUG_NAME("Phase_TurnEnd"), Event_enum);} break;
        case 7: {RECORD_DEBUG_EVENT(DEBUG_NAME("Phase_GameOver"), Event_enum);} break;
	}
}


inline void
RecordDebugValue_(render_settings *ValuePtr, char *GUID, u32 Depth)
{
	if(Depth > MAX_CIRCULAR_REFERENCE_DEPTH) return;
    DEBUG_VALUE_NAME(&ValuePtr->Resolution, "Resolution");
    DEBUG_VALUE_NAME(&ValuePtr->SwapInterval, "SwapInterval");
    DEBUG_VALUE_NAME(&ValuePtr->RenderScale, "RenderScale");
    DEBUG_VALUE_NAME(&ValuePtr->Bloom, "Bloom");
    DEBUG_VALUE_NAME(&ValuePtr->DesiredDepthPeelCount, "DesiredDepthPeelCount");
    DEBUG_VALUE_NAME(&ValuePtr->DesiredMultisamples, "DesiredMultisamples");
}

inline void
RecordDebugValue_(camera *ValuePtr, char *GUID, u32 Depth)
{
	if(Depth > MAX_CIRCULAR_REFERENCE_DEPTH) return;
    {DEBUG_DATA_BLOCK("Type");
        DEBUG_VALUE_NAME(&ValuePtr->Type, "Type");
    }
    DEBUG_VALUE_NAME(&ValuePtr->HorizontalFOV, "HorizontalFOV");
    DEBUG_VALUE_NAME(&ValuePtr->AspectRatio, "AspectRatio");
    DEBUG_VALUE_NAME(&ValuePtr->HalfWidth, "HalfWidth");
    DEBUG_VALUE_NAME(&ValuePtr->HalfHeight, "HalfHeight");
    DEBUG_VALUE_NAME(&ValuePtr->Near, "Near");
    DEBUG_VALUE_NAME(&ValuePtr->Far, "Far");
    DEBUG_VALUE_NAME(&ValuePtr->P, "P");
    DEBUG_VALUE_NAME(&ValuePtr->Rotation, "Rotation");
    DEBUG_VALUE_NAME(&ValuePtr->Projection_, "Projection_");
    DEBUG_VALUE_NAME(&ValuePtr->View_, "View_");
    DEBUG_VALUE_NAME(&ValuePtr->InvView_, "InvView_");
}

inline void
RecordDebugValue_(shape *ValuePtr, char *GUID, u32 Depth)
{
	if(Depth > MAX_CIRCULAR_REFERENCE_DEPTH) return;
    {DEBUG_DATA_BLOCK("Type");
        DEBUG_VALUE_NAME(&ValuePtr->Type, "Type");
    }
    {DEBUG_DATA_BLOCK("Sphere");
        DEBUG_VALUE_NAME(&ValuePtr->Sphere, "Sphere");
    }
    {DEBUG_DATA_BLOCK("Capsule");
        DEBUG_VALUE_NAME(&ValuePtr->Capsule, "Capsule");
    }
    {DEBUG_DATA_BLOCK("ConvexHull");
        if(ValuePtr->ConvexHull) {RecordDebugValue_(ValuePtr->ConvexHull, DEBUG_NAME("ConvexHull"), Depth+1);}
    }
}

inline void
RecordDebugValue_(collider *ValuePtr, char *GUID, u32 Depth)
{
	if(Depth > MAX_CIRCULAR_REFERENCE_DEPTH) return;
    {DEBUG_DATA_BLOCK("Next");
        if(ValuePtr->Next) {RecordDebugValue_(ValuePtr->Next, DEBUG_NAME("Next"), Depth+1);}
    }
    {DEBUG_DATA_BLOCK("Prev");
        if(ValuePtr->Prev) {RecordDebugValue_(ValuePtr->Prev, DEBUG_NAME("Prev"), Depth+1);}
    }
    DEBUG_VALUE_NAME(&ValuePtr->Mass, "Mass");
    DEBUG_VALUE_NAME(&ValuePtr->LocalInertialTensor, "LocalInertialTensor");
    DEBUG_VALUE_NAME(&ValuePtr->LocalCentroid, "LocalCentroid");
    {DEBUG_DATA_BLOCK("Shape");
        DEBUG_VALUE_NAME(&ValuePtr->Shape, "Shape");
    }
}

inline void
RecordDebugValue_(rigid_body *ValuePtr, char *GUID, u32 Depth)
{
	if(Depth > MAX_CIRCULAR_REFERENCE_DEPTH) return;
    DEBUG_VALUE_NAME(&ValuePtr->OnSolidGround, "OnSolidGround");
    DEBUG_VALUE_NAME(&ValuePtr->Collideable, "Collideable");
    {DEBUG_DATA_BLOCK("MoveSpec");
        DEBUG_VALUE_NAME(&ValuePtr->MoveSpec, "MoveSpec");
    }
    DEBUG_VALUE_NAME(&ValuePtr->InvMass, "InvMass");
    DEBUG_VALUE_NAME(&ValuePtr->LocalInvInertialTensor, "LocalInvInertialTensor");
    DEBUG_VALUE_NAME(&ValuePtr->WorldCentroid, "WorldCentroid");
    DEBUG_VALUE_NAME(&ValuePtr->LocalCentroid, "LocalCentroid");
    {DEBUG_DATA_BLOCK("ColliderSentinel");
        DEBUG_VALUE_NAME(&ValuePtr->ColliderSentinel, "ColliderSentinel");
    }
    {DEBUG_DATA_BLOCK("AABBNode");
        if(ValuePtr->AABBNode) {RecordDebugValue_(ValuePtr->AABBNode, DEBUG_NAME("AABBNode"), Depth+1);}
    }
    DEBUG_VALUE_NAME(&ValuePtr->IsInitialized, "IsInitialized");
}

inline void
RecordDebugValue_(convex_hull *ValuePtr, char *GUID, u32 Depth)
{
	if(Depth > MAX_CIRCULAR_REFERENCE_DEPTH) return;
    DEBUG_VALUE_NAME(&ValuePtr->VertexCount, "VertexCount");
    DEBUG_VALUE_NAME(ValuePtr->Vertices, "Vertices");
    DEBUG_VALUE_NAME(&ValuePtr->EdgeCount, "EdgeCount");
    {DEBUG_DATA_BLOCK("Edges");
        if(ValuePtr->Edges) {RecordDebugValue_(ValuePtr->Edges, DEBUG_NAME("Edges"), Depth+1);}
    }
    DEBUG_VALUE_NAME(&ValuePtr->FaceCount, "FaceCount");
    {DEBUG_DATA_BLOCK("Faces");
        if(ValuePtr->Faces) {RecordDebugValue_(ValuePtr->Faces, DEBUG_NAME("Faces"), Depth+1);}
    }
}

inline void
RecordDebugValue_(entity *ValuePtr, char *GUID, u32 Depth)
{
	if(Depth > MAX_CIRCULAR_REFERENCE_DEPTH) return;
    {DEBUG_DATA_BLOCK("ID");
        DEBUG_VALUE_NAME(&ValuePtr->ID, "ID");
    }
    {DEBUG_DATA_BLOCK("Info");
        if(ValuePtr->Info) {RecordDebugValue_(ValuePtr->Info, DEBUG_NAME("Info"), Depth+1);}
    }
    DEBUG_VALUE_NAME(&ValuePtr->P, "P");
    DEBUG_VALUE_NAME(&ValuePtr->Rotation, "Rotation");
    DEBUG_VALUE_NAME(&ValuePtr->InvRotation, "InvRotation");
    DEBUG_VALUE_NAME(&ValuePtr->dP, "dP");
    DEBUG_VALUE_NAME(&ValuePtr->AngularVelocity, "AngularVelocity");
    {DEBUG_DATA_BLOCK("RigidBody");
        DEBUG_VALUE_NAME(&ValuePtr->RigidBody, "RigidBody");
    }
    {DEBUG_DATA_BLOCK("Animation");
        if(ValuePtr->Animation) {RecordDebugValue_(ValuePtr->Animation, DEBUG_NAME("Animation"), Depth+1);}
    }
    {DEBUG_DATA_BLOCK("BaseAnimation");
        DEBUG_VALUE_NAME(&ValuePtr->BaseAnimation, "BaseAnimation");
    }
    {DEBUG_DATA_BLOCK("Owner");
        DEBUG_VALUE_NAME(&ValuePtr->Owner, "Owner");
    }
    DEBUG_VALUE_NAME(&ValuePtr->Color, "Color");
    DEBUG_VALUE_NAME(&ValuePtr->Flags, "Flags");
    {DEBUG_DATA_BLOCK("Attack");
        DEBUG_VALUE_NAME(&ValuePtr->Attack, "Attack");
    }
    {DEBUG_DATA_BLOCK("Defense");
        DEBUG_VALUE_NAME(&ValuePtr->Defense, "Defense");
    }
    {DEBUG_DATA_BLOCK("MaxDefense");
        DEBUG_VALUE_NAME(&ValuePtr->MaxDefense, "MaxDefense");
    }
    {DEBUG_DATA_BLOCK("Influence");
        DEBUG_VALUE_NAME(&ValuePtr->Influence, "Influence");
    }
    {DEBUG_DATA_BLOCK("VictoryPoints");
        DEBUG_VALUE_NAME(&ValuePtr->VictoryPoints, "VictoryPoints");
    }
    DEBUG_VALUE_NAME(&ValuePtr->BoardPosition, "BoardPosition");
    {DEBUG_DATA_BLOCK("Gold");
        DEBUG_VALUE_NAME(&ValuePtr->Gold, "Gold");
    }
    {DEBUG_DATA_BLOCK("Charges");
        DEBUG_VALUE_NAME(&ValuePtr->Charges, "Charges");
    }
    DEBUG_VALUE_NAME(&ValuePtr->ActionsRemaining, "ActionsRemaining");
    DEBUG_VALUE_NAME(&ValuePtr->AbilityCount, "AbilityCount");
    {DEBUG_DATA_BLOCK("Abilities");
        if(ValuePtr->Abilities) {RecordDebugValue_(ValuePtr->Abilities, DEBUG_NAME("Abilities"), Depth+1);}
    }
    DEBUG_VALUE_NAME(&ValuePtr->PieceCount, "PieceCount");
    {DEBUG_DATA_BLOCK("Pieces");
        if(ValuePtr->Pieces) {RecordDebugValue_(ValuePtr->Pieces, DEBUG_NAME("Pieces"), Depth+1);}
    }
}