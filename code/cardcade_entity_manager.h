/*@H
* File: cardcade_entity_manager.h
* Author: Jesse Calvert
* Created: November 5, 2018, 13:56
* Last modified: December 17, 2018, 14:39
*/

#pragma once

enum entity_flags
{
	EntityFlag_Traversable = (1 << 0),
	EntityFlag_OnBoard = (1 << 1),
	EntityFlag_InHand = (1 << 2),
	EntityFlag_IsBuilding = (1 << 3),
	EntityFlag_IsUnit = (1 << 4),
};

INTROSPECT()
struct entity
{
	entity_id ID;
	entity_info *Info;

	v3 P;
	quaternion Rotation;
	quaternion InvRotation;

	v3 dP;
	v3 AngularVelocity;

	rigid_body RigidBody;

	animation *Animation;
	animation BaseAnimation;

	player_name Owner;
	v4 Color;
	// TODO: we want this to be flag32(entity_flags) but the preprocessor complains.
	u32 Flags;

	s32 Attack;
	s32 Defense;
	s32 MaxDefense;
	s32 Influence;
	s32 VictoryPoints;

	v2i BoardPosition;
	s32 Gold;
	s32 Charges;
	u32 ActionsRemaining;

	u32 AbilityCount;
	ability Abilities[MAX_ABILITIES];

	u32 PieceCount;
	entity_visible_piece Pieces[MAX_VISIBLE_PIECES];
};

struct entity_hash_entry
{
	entity Entity;

	entity_hash_entry *NextInHash;

	entity_hash_entry *Next;
	entity_hash_entry *Prev;
};

struct entity_manager
{
	memory_region *Region;

	entity_hash_entry *EntityHashTable[1024];
	entity_hash_entry *FirstFreeEntityHashEntry;
	u32 NextEntityID;
	u32 EntityCount;
	entity_hash_entry HashEntrySentinel;

	physics_state *PhysicsState;
	animator *Animator;
};

internal entity *AddEntity(entity_manager *EntityManager);
internal void RemoveEntity(entity_manager *EntityManager, entity *Entity);
internal entity *GetEntity(entity_manager *EntityManager, entity_id ID);
inline b32 IsValidID(entity_manager *EntityManager, entity_id ID);

inline entity *GetFirst(entity_manager *Manager)
{
	entity_hash_entry *HashEntry = Manager->HashEntrySentinel.Next;
	entity *Result = &HashEntry->Entity;
	return Result;
}

inline entity *GetNext(entity_manager *Manager, entity *Entity)
{
	entity_hash_entry *Entry = (entity_hash_entry *)Entity;
	entity_hash_entry *NextEntry = Entry->Next;
	entity *Result = 0;
	if(NextEntry != &Manager->HashEntrySentinel)
	{
		Result = &NextEntry->Entity;
	}

	return Result;
}

inline void
UpdateRotation(entity *Entity)
{
	Entity->Rotation = NormalizeIfNeeded(Entity->Rotation);
	Entity->InvRotation = Conjugate(Entity->Rotation);
}

inline void
UpdatePositionFromWorldCentroid(entity *Entity)
{
	Entity->P = Entity->RigidBody.WorldCentroid + RotateBy(-Entity->RigidBody.LocalCentroid, Entity->Rotation);
}

inline void
UpdateWorldCentroidFromPosition(entity *Entity)
{
	Entity->RigidBody.WorldCentroid = Entity->P + RotateBy(Entity->RigidBody.LocalCentroid, Entity->Rotation);
}

inline v3
LocalPointToWorldPoint(entity *Entity, v3 Point)
{
	v3 Result = RotateBy(Point, Entity->Rotation) + Entity->P;
	return Result;
}

inline v3
WorldPointToLocalPoint(entity *Entity, v3 Point)
{
	v3 Result = RotateBy(Point - Entity->P, Entity->InvRotation);
	return Result;
}

inline v3
LocalVectorToWorldVector(entity *Entity, v3 Vector)
{
	v3 Result = RotateBy(Vector, Entity->Rotation);
	return Result;
}

inline v3
WorldVectorToLocalVector(entity *Entity, v3 Vector)
{
	v3 Result = RotateBy(Vector, Entity->InvRotation);
	return Result;
}

inline b32
IsPhysicsEnabled(entity *Entity)
{
	b32 Result = ((Entity->Animation == 0) && (Entity->BaseAnimation.Type == Animation_Physics));
	return Result;
}

inline void
ApplyTorque(entity *Entity, v3 Torque)
{
	if((Determinant(Entity->RigidBody.LocalInvInertialTensor) != 0.0f) &&
	   IsPhysicsEnabled(Entity))
	{
		Entity->AngularVelocity += RotateBy(Entity->RigidBody.LocalInvInertialTensor*RotateBy(Torque, Entity->InvRotation), Entity->Rotation);
	}
}

inline void
ApplyImpulse(entity *Entity, v3 Impulse, v3 Location)
{
	if((Entity->RigidBody.InvMass) &&
	   IsPhysicsEnabled(Entity))
	{
		v3 RelativeLocation = Location - Entity->RigidBody.WorldCentroid;

		Entity->dP += Entity->RigidBody.InvMass*Impulse;

		v3 TorqueImpulse = Cross(RelativeLocation, Impulse);
		ApplyTorque(Entity, TorqueImpulse);
	}
}
