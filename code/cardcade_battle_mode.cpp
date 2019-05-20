/*@H
* File: cardcade_battle_mode.cpp
* Author: Jesse Calvert
* Created: May 21, 2018, 14:49
* Last modified: December 17, 2018, 14:59
*/

internal entity *
PickEntityRayCast(game_mode_battle *BattleMode, v3 Origin, v3 Direction)
{
	entity *Result = 0;
	ray_cast_result RayCast = AABBRayCastQuery(&BattleMode->Physics.DynamicAABBTree, Origin, Direction);
	if(RayCast.RayHit && RayCast.EntityHit)
	{
		Result = RayCast.EntityHit;
	}

	return Result;
}

internal entity *
AddPhysicsBox(game_state *GameState, game_mode_battle *BattleMode, v3 P, v3 Dim)
{
	entity *Result = AddEntity(BattleMode->EntityManager);
	Result->P = P;
	Result->Rotation = Q(1,0,0,0);

	Result->BaseAnimation.Type = Animation_Physics;

	collider Sentinel = {};
	DLIST_INIT(&Sentinel);
	collider *Collider0 = BuildBoxCollider(&GameState->FrameRegion, &BattleMode->Physics, V3(0,0,0), Dim, Q(1,0,0,0));
	// collider *Collider0 = BuildSphereCollider(&BattleMode->Physics, V3(0,0,0), Dim.x);
	// collider *Collider0 = BuildCapsuleCollider(&BattleMode->Physics, V3(0,0,0), 0.5f*Dim.x, Dim.x, Q(1,0,0,0));
	DLIST_INSERT(&Sentinel, Collider0);

	move_spec MoveSpec = DefaultMoveSpec();
	InitializeRigidBody(&BattleMode->Physics, Result, &Sentinel, true, MoveSpec);

	surface_material Material = {};
	Material.Roughness = 0.5f;
	Material.Metalness = 0.8f;
	Material.Color = V4(0.4f, 0.5f, 0.8f, 1.0f);

	AddPieceMesh(Result, Dim, NoRotation(), {}, {0}, GetMeshID(GameState->Assets, Asset_RoundedBox), Material);

	return Result;
}

internal b32
EntityIsOnGround(entity *Entity)
{
	b32 Result = Entity->RigidBody.OnSolidGround;
	return Result;
}

internal void
AddDecal(game_mode_battle *BattleMode, v3 P, v3 N)
{
	Assert(BattleMode->DecalCount < ArrayCount(BattleMode->Decals));
	oriented_box *NewDecalBox = BattleMode->Decals + BattleMode->DecalCount++;

	v3 Dim = V3(2.0f, 2.0f, 0.05f);

	v3 Up = V3(0,1,0);
	v3 Right = V3(0,0,1);
	v3 Z = N;
	v3 X = NormalizeOr(Cross(Up, Z), Right);
	v3 Y = Cross(Z, X);
	quaternion Rotation = RotationQuaternion(X, Y, Z);

	NewDecalBox->P = P;
	NewDecalBox->Dim = Dim;
	NewDecalBox->Rotation = Rotation;
}

internal void
BeginBoardLists(entity_manager *Manager, board *Board)
{
	u32 TileCount = Board->Width*Board->Height;
	for(u32 TileIndex = 0;
		TileIndex < TileCount;
		++TileIndex)
	{
		board_tile *Tile = Board->Tiles + TileIndex;
		Tile->List.EntityCount = 0;
	}

	FOR_EACH(Entity, Manager, entity)
	{
		if(Entity->Flags & EntityFlag_OnBoard)
		{
			board_tile *Tile = GetTile(Board, Entity->BoardPosition);
			entity_list *List = &Tile->List;
			Assert(List->EntityCount < ArrayCount(List->EntityIDs));
			List->EntityIDs[List->EntityCount++] = Entity->ID;
		}
	}
}

internal void
EndBoardLists(board *Board)
{

}

internal void
SetupBoard(game_state *GameState, game_mode_battle *BattleMode, board *Board)
{
	u32 Radius = 6;
	Board->Width = Radius*2 - 1;
	Board->Height = Radius*2 - 1;
	Board->Tiles = PushArray(&GameState->ModeRegion, Board->Width*Board->Height, board_tile);

	f32 Advance = BattleMode->GroundSpacing + BattleMode->GroundDim.x;
	v3 Center = V3(0,0,0);
	v3 Left = Center - V3((Board->Width - 1)*Advance, 0, 0);
	f32 HexShift = 0.5f*(BattleMode->GroundSpacing + BattleMode->GroundDim.x);
	for(u32 Row = 0;
		Row < Board->Height;
		++Row)
	{
		v3 P = Left + ((f32)Row)*V3(Advance, 0, -HexShift);
		for(u32 Column = 0;
			Column < Board->Width;
			++Column)
		{
			board_tile *Tile = Board->Tiles + (Board->Width * Row) + Column;
			Tile->P = P;
			Tile->BoardPosition = V2i(Column, Row);
			Tile->Dim = BattleMode->GroundDim;

			if((Row + Column < (Radius - 1)) ||
			   (Row + Column > (3*Radius - 3)))
			{
				// NOTE: Off of board.
			}
			else
			{
#if 0
				// TODO: Don't use game entropy for this?
				u32 Choice = RandomChoice(&BattleMode->Entropy, 4);
				switch(Choice)
				{
					case 0:
					{
						AddGround(GameState, BattleMode, Tile->BoardPosition);
					} break;

					case 1:
					{
						AddGround(GameState, BattleMode, Tile->BoardPosition);
						AddForest(GameState, BattleMode, Tile->BoardPosition);
					} break;

					case 2:
					{
						AddGround(GameState, BattleMode, Tile->BoardPosition);
						AddMountains(GameState, BattleMode, Tile->BoardPosition);
					} break;

					case 3:
					{
						AddWater(GameState, BattleMode, Tile->BoardPosition);
					} break;
				}

#else
				AddGround(GameState, BattleMode, Tile->BoardPosition);

#endif
			}

			P += V3(Advance, 0, HexShift);
		}
	}
}

internal void
InitDeck(game_state	*GameState, game_mode_battle *BattleMode, player_name Player, u32 DeckIndex)
{
	deck *Deck = BattleMode->Decks + Player;
	Deck->CardCount = 0;

	switch(DeckIndex)
	{
		case 0:
		{
			Deck->Cards[Deck->CardCount++] = Ability_BuildBank;
			Deck->Cards[Deck->CardCount++] = Ability_BuildBank;
			Deck->Cards[Deck->CardCount++] = Ability_BuildBank;
			Deck->Cards[Deck->CardCount++] = Ability_BuildRiskInvestment;
			Deck->Cards[Deck->CardCount++] = Ability_BuildRiskInvestment;
			Deck->Cards[Deck->CardCount++] = Ability_BuildRiskInvestment;
			Deck->Cards[Deck->CardCount++] = Ability_BuildGoldMine;
			Deck->Cards[Deck->CardCount++] = Ability_BuildGoldMine;
			Deck->Cards[Deck->CardCount++] = Ability_BuildGoldMine;
			Deck->Cards[Deck->CardCount++] = Ability_SummonWarrior;
			Deck->Cards[Deck->CardCount++] = Ability_SummonWarrior;
			Deck->Cards[Deck->CardCount++] = Ability_SummonWarrior;
			Deck->Cards[Deck->CardCount++] = Ability_SummonMercenaries;
			Deck->Cards[Deck->CardCount++] = Ability_SummonMercenaries;
			Deck->Cards[Deck->CardCount++] = Ability_SummonMercenaries;
			Deck->Cards[Deck->CardCount++] = Ability_HedgeFund;
			Deck->Cards[Deck->CardCount++] = Ability_HedgeFund;
			Deck->Cards[Deck->CardCount++] = Ability_HedgeFund;
			Deck->Cards[Deck->CardCount++] = Ability_Buyout;
			Deck->Cards[Deck->CardCount++] = Ability_Buyout;
			Deck->Cards[Deck->CardCount++] = Ability_Buyout;
			Deck->Cards[Deck->CardCount++] = Ability_Bribe;
			Deck->Cards[Deck->CardCount++] = Ability_Bribe;
			Deck->Cards[Deck->CardCount++] = Ability_Bribe;
			Deck->Cards[Deck->CardCount++] = Ability_Fortify;
			Deck->Cards[Deck->CardCount++] = Ability_Fortify;
			Deck->Cards[Deck->CardCount++] = Ability_Fortify;
			Deck->Cards[Deck->CardCount++] = Ability_BuildPowerPlant;
			Deck->Cards[Deck->CardCount++] = Ability_BuildPowerPlant;
			Deck->Cards[Deck->CardCount++] = Ability_BuildPowerPlant;
		} break;

		InvalidDefaultCase;
	}

	Assert(Deck->CardCount < ArrayCount(Deck->Cards));
}

internal void
RenderEntityInfo(ui_state *UI, entity *Entity, v2i Min, v2i Max)
{
	render_group *Group = UI->Group;

	TemporaryClipRectChange(Group, Min, Max);
	TemporaryRenderFlagChange(Group,
		RenderFlag_SDF|RenderFlag_NotLighted|RenderFlag_AlphaBlended|RenderFlag_TopMost);

	PushQuad(Group, V2iToV2(Min), V2iToV2(Max), V4(0.1f, 0.1f, 0.4f, 0.5f));

	rectangle2 LayoutRect = Rectangle2(V2iToV2(Min), V2iToV2(Max));
	ui_layout Layout = BeginUILayout(UI, Asset_TimesFont, 20.0f, V4(1,1,1,1), LayoutRect);

	if(Entity->Info->DisplayName.Length)
	{
		BeginRow(&Layout);
		Label(&Layout, Entity->Info->DisplayName);
		EndRow(&Layout);
	}

	Layout.Color = V4(1,1,0,1);
	if(Entity->Attack)
	{
		BeginRow(&Layout);
		Labelf(&Layout, "Attack : %d", Entity->Attack);
		EndRow(&Layout);
	}

	if(Entity->MaxDefense)
	{
		BeginRow(&Layout);
		Labelf(&Layout, "Defense : %d/%d", Entity->Defense, Entity->MaxDefense);
		EndRow(&Layout);
	}

	// if(Entity->Traversable)
	{
		BeginRow(&Layout);
		Labelf(&Layout, "Traversable : %s", (Entity->Flags & EntityFlag_Traversable) ? WrapZ("true") : WrapZ("false"));
		EndRow(&Layout);
	}

	if(Entity->Gold)
	{
		BeginRow(&Layout);
		Labelf(&Layout, "Gold : %d", Entity->Gold);
		EndRow(&Layout);
	}

	if(Entity->Charges)
	{
		BeginRow(&Layout);
		Labelf(&Layout, "Charges : %d", Entity->Charges);
		EndRow(&Layout);
	}

	if(Entity->Influence)
	{
		BeginRow(&Layout);
		Labelf(&Layout, "Influence : %d", Entity->Influence);
		EndRow(&Layout);
	}

	if(Entity->Owner)
	{
		BeginRow(&Layout);
		string OwnerString = WrapZ(Names_player_name[Entity->Owner]);
		Labelf(&Layout, "Owner : %s", OwnerString);
		EndRow(&Layout);
	}
	EndUILayout(&Layout);
}

internal v2
WorldCoordinatesToScreenCoordinates(camera *Camera, v3 WorldP, v2i ScreenResolution)
{
	v3 WorldRay = WorldP - Camera->P;
	v3 CameraRay = RotateBy(WorldRay, Conjugate(Camera->Rotation));
	CameraRay.x /= -CameraRay.z;
	CameraRay.y /= -CameraRay.z;
	f32 CameraWidth = 2.0f*Tan(0.5f*Camera->HorizontalFOV);
	f32 CameraHeight = CameraWidth/Camera->AspectRatio;
	v2 NormalizedScreenCoords = V2((CameraRay.x*ScreenResolution.x) / CameraWidth,
								   (CameraRay.y*ScreenResolution.y) / CameraHeight);
	v2 Result = NormalizedScreenCoords + 0.5f*ScreenResolution;

	return Result;
}

internal v3
ScreenPToWorldDirection(camera *Camera, v2 ScreenP, v2i ScreenResolution)
{
	v2 NormalizedP = ScreenP - 0.5f*ScreenResolution;
	f32 CameraWidth = 2.0f*Camera->Near*Tan(0.5f*Camera->HorizontalFOV);
	f32 CameraHeight = CameraWidth/Camera->AspectRatio;
	v3 CameraCoordsP = V3(NormalizedP.x*CameraWidth/ScreenResolution.x,
		NormalizedP.y*CameraHeight/ScreenResolution.y,
		-Camera->Near);
	v3 Result = Normalize(RotateBy(CameraCoordsP, Camera->Rotation));

	return Result;
}

internal ui_panel_positions
GetUIPanelPositions(v2i Dim, s32 Spacing, u32 PanelCount, v2i ScreenResolution, v2i LeftCenterP)
{
	ui_panel_positions Result = {};
	Result.Dim = Dim;
	Result.Spacing = Spacing;
	Result.LeftCenterP = LeftCenterP;
	v2i TotalDim = V2i(Result.Dim.x, PanelCount*(Result.Dim.y + Result.Spacing));
	v2i MinP = LeftCenterP + V2i(Result.Spacing, -TotalDim.y/2);
	MinP.x = Clamp(MinP.x, 0, ScreenResolution.x);
	MinP.y = Clamp(MinP.y, 0, ScreenResolution.y);
	v2i MaxP = MinP + TotalDim;
	MaxP.x = Clamp(MaxP.x, 0, ScreenResolution.x);
	MaxP.y = Clamp(MaxP.y, 0, ScreenResolution.y);
	MinP = MaxP - TotalDim;

	Result.At = V2i(MinP.x, MaxP.y - Result.Dim.y);
	Result.Advance = V2i(0, -(Result.Spacing + Result.Dim.y));

	return Result;
}

internal void
ChangeGamePhase(game_mode_battle *BattleMode, ui_state *UI, game_phase NewPhase)
{
	BattleMode->Phase = NewPhase;

	UI->SelectedID = {0};
	UI->SelectedAbilityIndex = U32Maximum;
	UI->TargetCount = 0;
}

internal void
UseAbilityIfReady(game_state *GameState, ui_state *UI, game_mode_battle *BattleMode)
{
	entity *CastingEntity = GetEntity(BattleMode->EntityManager, UI->SelectedID);

	if((CastingEntity->Owner == BattleMode->CurrentPlayer) &&
	   ((BattleMode->Phase == Phase_PlayerActions) || (BattleMode->Phase == Phase_PlaceHQ)))
	{
		entity *Player = GetEntity(BattleMode->EntityManager, BattleMode->Players[CastingEntity->Owner]);
		ability *Ability = CastingEntity->Abilities + UI->SelectedAbilityIndex;
		if(UI->TargetCount == Ability->TargetCount)
		{
			b32 ActionTaken = UseAbility(GameState, BattleMode, Player, CastingEntity, Ability, UI->TargetIDs);

			UI->SelectedID = {0};
			UI->TargetCount = 0;
			UI->SelectedAbilityIndex = U32Maximum;
		}
	}
	else
	{
		UI->SelectedID = {0};
		UI->TargetCount = 0;
		UI->SelectedAbilityIndex = U32Maximum;
	}
}

internal void
EndBattleModeUI(game_state *GameState, ui_state *UI, game_mode_battle *BattleMode, game_input *Input)
{
	if(Input->KeyStates[Key_LeftClick].WasReleased)
	{
		if(UI->NextInteraction == UI->HotInteraction)
		{
			ui_interaction *Interaction = &UI->NextInteraction;

			switch(Interaction->Type)
			{
				case UI_Interaction_PassTurn:
				{
					game_phase NextPhase = Phase_None;
					if(BattleMode->Phase == Phase_PlayerActions)
					{
						NextPhase = Phase_TurnEnd;
					}
					else if(BattleMode->Phase == Phase_PlaceHQ)
					{
						NextPhase = Phase_HQEnd;
					}

					if(NextPhase)
					{
						ChangeGamePhase(BattleMode, UI, NextPhase);
					}
				} break;

				case UI_Interaction_SelectAbility:
				{
					entity *CastingEntity = GetEntity(BattleMode->EntityManager, Interaction->EntityID);
					if(CastingEntity->Owner == BattleMode->CurrentPlayer)
					{
						ability *Ability = CastingEntity->Abilities + Interaction->AbilityIndex;
						if(Ability->Type == AbilityType_Activated)
						{
							UI->SelectedAbilityIndex = Interaction->AbilityIndex;
							UI->SelectedID = Interaction->EntityID;
							UI->TargetCount = 0;

							UseAbilityIfReady(GameState, UI, BattleMode);
						}
					}
				} break;

				case UI_Interaction_SelectEntity:
				{
					if(UI->SelectedAbilityIndex != U32Maximum)
					{
						entity *Target = GetEntity(BattleMode->EntityManager, Interaction->EntityID);
						if(Target)
						{
							entity *CastingEntity = GetEntity(BattleMode->EntityManager, UI->SelectedID);
							ability *Ability = CastingEntity->Abilities + UI->SelectedAbilityIndex;

							entity_id TargetID = Interaction->EntityID;
							b32 TargetValid = false;

							if(Target->Flags & EntityFlag_OnBoard)
							{
								entity_list List = GetEntitiesAtPosition(&BattleMode->Board, Target->BoardPosition);
								for(u32 TargetIndex = 0;
									TargetIndex < List.EntityCount;
									++TargetIndex)
								{
									entity_id ID = List.EntityIDs[TargetIndex];
									entity *PotentialTarget = GetEntity(BattleMode->EntityManager, ID);
									TargetValid = TargetIsValid(BattleMode, CastingEntity, Ability, PotentialTarget);
									if(TargetValid)
									{
										TargetID = ID;
										break;
									}
								}
							}
							else
							{
								TargetValid = TargetIsValid(BattleMode, CastingEntity, Ability, Target);
							}

							if(TargetValid)
							{
								Assert(UI->TargetCount < ArrayCount(UI->TargetIDs));
								UI->TargetIDs[UI->TargetCount++] = TargetID;
								UseAbilityIfReady(GameState, UI, BattleMode);
							}
						}
					}
					else
					{
						UI->SelectedID = {0};
						UI->SelectedAbilityIndex = U32Maximum;
						UI->TargetCount = 0;

						if(IsValidID(BattleMode->EntityManager, Interaction->EntityID))
						{
							// NOTE: If a tile has more than one entity owned by the player,
							//	they can only select one of them!
							entity *SelectedEntity = GetEntity(BattleMode->EntityManager, Interaction->EntityID);
							if(SelectedEntity->Flags & EntityFlag_OnBoard)
							{
								entity_list EntityList = GetEntitiesAtPosition(&BattleMode->Board, SelectedEntity->BoardPosition);
								player_name Opponent = (BattleMode->CurrentPlayer == Player_1) ? Player_2 : Player_1;
								for(u32 EntityIndex = 0;
									EntityIndex < EntityList.EntityCount;
									++EntityIndex)
								{
									entity_id ID = EntityList.EntityIDs[EntityIndex];
									entity *Entity = GetEntity(BattleMode->EntityManager, ID);
									if(Entity)
									{
										if(Entity->Owner == BattleMode->CurrentPlayer)
										{
											UI->SelectedID = Entity->ID;
											break;
										}
										else if(Entity->Owner == Opponent)
										{
											UI->SelectedID = Entity->ID;
										}
									}
								}
							}
							else if(SelectedEntity->Flags & EntityFlag_InHand)
							{
								UI->SelectedID = SelectedEntity->ID;
								UI->SelectedAbilityIndex = 0;

								UseAbilityIfReady(GameState, UI, BattleMode);
							}
						}
					}
				} break;
			}
		}
	}

	EndFrameUI(UI);
}

internal void
PushEntityHighlight(render_group *RenderGroup, game_mode_battle *BattleMode, entity *Entity, v4 Color)
{
	if(Entity->Flags & EntityFlag_OnBoard)
	{
		v3 HalfDim = 0.5f*BattleMode->GroundDim;
		quaternion Rotation = RotationQuaternion(V3(1,0,0), -0.5f*Pi32);
		f32 Width = 0.1f;
		f32 Roughness = 1.0f;
		f32 Metalness = 0.0f;
		PushTexture(RenderGroup, Entity->P + V3(HalfDim.x, HalfDim.y + 0.01f, 0.0f),
			Rotation, V2(Width, 2.0f*HalfDim.z),
			RenderGroup->WhiteTexture, Roughness, Metalness, Color);
		PushTexture(RenderGroup, Entity->P + V3(-HalfDim.x, HalfDim.y + 0.01f, 0.0f),
			Rotation, V2(Width, 2.0f*HalfDim.z),
			RenderGroup->WhiteTexture, Roughness, Metalness, Color);
		PushTexture(RenderGroup, Entity->P + V3(0.0f, HalfDim.y + 0.01f, HalfDim.z),
			Rotation, V2(2.0f*HalfDim.x, Width),
			RenderGroup->WhiteTexture, Roughness, Metalness, Color);
		PushTexture(RenderGroup, Entity->P + V3(0.0f, HalfDim.y + 0.01f, -HalfDim.z),
			Rotation, V2(2.0f*HalfDim.x, Width),
			RenderGroup->WhiteTexture, Roughness, Metalness, Color);
	}
	else if(Entity->Flags & EntityFlag_InHand)
	{
		v3 HalfDim = 0.5f*BattleMode->CardDim;
		quaternion Rotation = Entity->Rotation;
		f32 Width = 0.1f;
		f32 Roughness = 1.0f;
		f32 Metalness = 0.0f;
		PushTexture(RenderGroup, LocalPointToWorldPoint(Entity, V3(HalfDim.x, 0.0f, 0.0f)),
			Rotation, V2(Width, 2.0f*HalfDim.y),
			RenderGroup->WhiteTexture, Roughness, Metalness, Color);
		PushTexture(RenderGroup, LocalPointToWorldPoint(Entity, V3(-HalfDim.x, 0.0f, 0.0f)),
			Rotation, V2(Width, 2.0f*HalfDim.y),
			RenderGroup->WhiteTexture, Roughness, Metalness, Color);
		PushTexture(RenderGroup, LocalPointToWorldPoint(Entity, V3(0.0f, HalfDim.y, 0.0f)),
			Rotation, V2(2.0f*HalfDim.x, Width),
			RenderGroup->WhiteTexture, Roughness, Metalness, Color);
		PushTexture(RenderGroup, LocalPointToWorldPoint(Entity, V3(0.0f, -HalfDim.y, 0.0f)),
			Rotation, V2(2.0f*HalfDim.x, Width),
			RenderGroup->WhiteTexture, Roughness, Metalness, Color);
	}
}

internal void
SetupPlayers(game_state *GameState, game_mode_battle *BattleMode)
{
	v4 PlayerColors[Player_Count] = {};
	PlayerColors[Player_Neutral] = V4(1,1,1,1);
	PlayerColors[Player_1] = V4(1,0.5f,0.5f,1);
	PlayerColors[Player_2] = V4(0.5f,0.5f,1,1);

	for(u32 Index = Player_Neutral;
		Index < ArrayCount(BattleMode->Players);
		++Index)
	{
		entity *Player = AddEntity(BattleMode->EntityManager);
		BattleMode->Players[Index] = Player->ID;
		Player->Owner = (player_name)Index;
		Player->Gold = 5;
		Player->Color = PlayerColors[Player->Owner];
	}
}

internal void
PlayBattleMode(game_state *GameState)
{
	SetGameMode(GameState, GameMode_BattleMode);

	game_mode_battle *BattleMode = GameState->BattleMode = PushStruct(&GameState->ModeRegion, game_mode_battle);
	RandomSeriesSeed(&BattleMode->Entropy);

	BattleMode->GroundDim = V3(1.0, 0.1f, 1.0f);
	BattleMode->CardDim = V3(1.0f, 1.3f, 0.01f);
	BattleMode->GroundSpacing = 0.1f;
	BattleMode->ActionsPerTurn = 4;
	BattleMode->PointsToWin = 4;

	BattleMode->EntityManager = PushStruct(&GameState->ModeRegion, entity_manager);
	InitEntityManager(BattleMode, BattleMode->EntityManager, &GameState->ModeRegion, GameState->Assets);
	InitializePhysics(&BattleMode->Physics, BattleMode->EntityManager, &GameState->ModeRegion);
	InitAnimator(&BattleMode->Animator, &BattleMode->Physics, &GameState->ModeRegion);
	BattleMode->EntityManager->PhysicsState = &BattleMode->Physics;
	BattleMode->EntityManager->Animator = &BattleMode->Animator;

	BattleMode->ParticleSystem = PushStruct(&GameState->ModeRegion, particle_system, AlignNoClear(16));
	InitializeParticleSystem(BattleMode->ParticleSystem, GameState->Assets);

	BattleMode->WorldCamera.Type = Camera_Perspective;
	f32 WorldNear = 0.01f;
	f32 WorldFar = 50.0f;
	f32 WorldHorizontalFOV = 90.0f*(Pi32/180.0f);
	f32 AspectRatio = 16.0f/9.0f;

	ChangeCameraSettings(&BattleMode->WorldCamera, WorldHorizontalFOV, AspectRatio,
		WorldNear, WorldFar);
	MoveCamera(&BattleMode->WorldCamera, V3(0, 0, 0));
	RotateCamera(&BattleMode->WorldCamera, Q(1,0,0,0));

	InitDeck(GameState, BattleMode, Player_1, 0);
	InitDeck(GameState, BattleMode, Player_2, 0);

	InitAbilities(&GameState->ModeRegion, BattleMode->Abilities);
	ChangeGamePhase(BattleMode, &BattleMode->UI, Phase_GameBegin);
}

internal void
UpdateAndRenderEntities(game_state *GameState, game_mode_battle *BattleMode,
	render_group *RenderGroup, v2i ScreenResolution, f32 Simdt)
{
	camera *WorldCamera = &BattleMode->WorldCamera;
	ui_state *UI = &BattleMode->UI;

	u32 CardHandCount = 0;
	f32 HandDistance = 8.0f;
	v3 CardStartDirection = ScreenPToWorldDirection(WorldCamera,
		V2(0.25f*ScreenResolution.x, 100.0f),
		ScreenResolution);
	v3 CardStart = WorldCamera->P + HandDistance*CardStartDirection;
	v3 CardAdvance = RotateBy(V3(1.1f*BattleMode->CardDim.x, 0, 0), WorldCamera->Rotation);
	FOR_EACH(Entity, BattleMode->EntityManager, entity)
	{
		if(Entity->Flags & EntityFlag_InHand)
		{
			if(Entity->Owner == BattleMode->CurrentPlayer)
			{
				v3 P = CardStart + ((f32)CardHandCount)*CardAdvance;
				quaternion Rotation = WorldCamera->Rotation;
				if(Entity->BaseAnimation.Type == Animation_Spring)
				{
					spring_animation *SpringAnim = &Entity->BaseAnimation.SpringAnim;
					SpringAnim->TargetP = P;
					SpringAnim->TargetRotation = Rotation;
				}
				else
				{
					Entity->P = P;
					Entity->Rotation = Rotation;
					UpdateRotation(Entity);
				}

				++CardHandCount;
			}
			else
			{
				v3 P = V3(0, 15.0f, 0);
				quaternion Rotation = WorldCamera->Rotation * RotationQuaternion(V3(0,1,0), Pi32);
				if(Entity->BaseAnimation.Type == Animation_Spring)
				{
					spring_animation *SpringAnim = &Entity->BaseAnimation.SpringAnim;
					SpringAnim->TargetP = P;
					SpringAnim->TargetRotation = Rotation;
				}
				else
				{
					Entity->P = P;
					Entity->Rotation = Rotation;
					UpdateRotation(Entity);
				}
			}
		}

		AnimateEntity(&BattleMode->Animator, Entity, Simdt);

		rigid_body *Body = &Entity->RigidBody;
		if(Body->IsInitialized)
		{
			UpdateWorldCentroidFromPosition(Entity);
			UpdateRotation(Entity);
			AABBUpdate(&BattleMode->Physics.DynamicAABBTree, Entity);
		}
	}

	PhysicsFindAndResolveConstraints(GameState, &BattleMode->Physics, BattleMode->EntityManager, Simdt);

#if 0
	// if(Input->KeyStates[Key_F].Pressed)
	{
		// NOTE: Particle test
		SpawnFire(BattleMode->ParticleSystem, V3(0,0,0));
	}
#endif

	//
	// NOTE: Render
	//

	v3 LightColor = V3(1.0f, 1.0f, 1.0f);
	f32 Intensity = 2.0f;
	v3 WorldLightDirection = V3(0.5f, -1.5f, -1.0f);
	f32 LightArcDegrees = 0.0f;
	PushDirectionalLight(RenderGroup, WorldLightDirection, LightColor, Intensity, LightArcDegrees);
	PushAmbientLight(RenderGroup, 0.03f, V3(1,1,1));

	SetCamera(RenderGroup, WorldCamera);

	FOR_EACH(Entity, BattleMode->EntityManager, entity)
	{
		b32 Highlight = false;
		v4 HighlightColor = V4(1,1,1,1);

		if(UI->SelectedAbilityIndex < U32Maximum)
		{
			entity *CastingEntity = GetEntity(BattleMode->EntityManager, UI->SelectedID);
			ability *Ability = CastingEntity->Abilities + UI->SelectedAbilityIndex;

			if(TargetIsValid(BattleMode, CastingEntity, Ability, Entity))
			{
				Highlight = true;
				HighlightColor = V4(1,0.5f,0.5f,1);
			}
		}

		if(Entity->ID == UI->SelectedID)
		{
			Highlight = true;
			HighlightColor = V4(0.5f,1.0f,0.5f,1);
		}

		if(Entity->ID == BattleMode->HighlightID)
		{
			Highlight = true;
			HighlightColor = V4(1,1,1,1);
		}

		if(Highlight)
		{
			PushEntityHighlight(RenderGroup, BattleMode, Entity, HighlightColor);
		}

		if(Entity->MaxDefense && (Entity->Defense <= 0))
		{
			SpawnFire(BattleMode->ParticleSystem, Entity->P);
		}

		entity *Owner = GetEntity(BattleMode->EntityManager, BattleMode->Players[Entity->Owner]);
		for(u32 PieceIndex = 0;
			PieceIndex < Entity->PieceCount;
			++PieceIndex)
		{
			entity_visible_piece *Piece = Entity->Pieces + PieceIndex;
			v3 RenderP = LocalPointToWorldPoint(Entity, Piece->Offset);
			quaternion RenderRotation = Entity->Rotation*Piece->Rotation;
			surface_material *Material = &Piece->Material;
			v4 Color = Owner ? Owner->Color : Material->Color;

			switch(Piece->Type)
			{
				case PieceType_Cube:
				{
					PushCube(RenderGroup, RenderP, Piece->Dim, RenderRotation,
						Material->Roughness, Material->Metalness, Color);
				} break;

				case PieceType_Bitmap:
				{
					PushTexture(RenderGroup, RenderP, RenderRotation, Piece->Dim.xy,
						Piece->BitmapID, Material->Roughness, Material->Metalness, Color);
				} break;

				case PieceType_Bitmap_Upright:
				{
					PushUprightTexture(RenderGroup, RenderP, Piece->Dim.xy,
						Piece->BitmapID, Material->Roughness, Material->Metalness, Color);
				} break;

				case PieceType_Mesh:
				{
					PushMesh(RenderGroup, RenderP, RenderRotation, Piece->Dim,
						Piece->MeshID, Piece->BitmapID, Piece->Material);
				} break;

				case PieceType_EntityInfo:
				{
					PushTexture(RenderGroup, RenderP, RenderRotation, Piece->Dim.xy, RenderGroup->WhiteTexture,
						1.0f, 0.0f, V4(1,1,1,1));

					font_id Font = GetFontID(GameState->Assets, Asset_TimesFont);
					f32 FontHeight = 0.15f;
					f32 Border = 0.5f*FontHeight;
					v4 TextColor = V4(0,0,0,1);
					v2 TextAt = V2(Border + -0.5f*Piece->Dim.x, 0.5f*Piece->Dim.y - FontHeight);

					ability *Ability = Entity->Abilities + 0;
					Assert(Ability);

					v3 Normal = RotateBy(V3(0,0,1), RenderRotation);
					v3 Down = RotateBy(V3(0,-1,0), RenderRotation);
					v3 Advance = Cross(Normal, Down);
					v3 At = RenderP + 0.05f*Normal + TextAt.x*Advance + -TextAt.y*Down;

					TemporaryRenderFlagChange(RenderGroup, RenderFlag_SDF);
					char Buffer[128];
					string CardText = {};

					CardText = Ability->DisplayName;
					PushTextBillboard(RenderGroup, Font, FontHeight, At, Normal, Down, CardText, TextColor);
					At += FontHeight * Down;

					if(Ability->Cost.Gold)
					{
						CardText = FormatString(Buffer, ArrayCount(Buffer), "Gold : %d", Ability->Cost.Gold);
						PushTextBillboard(RenderGroup, Font, FontHeight, At, Normal, Down, CardText, TextColor);
						At += FontHeight * Down;
					}
					if(Ability->Cost.Charges)
					{
						CardText = FormatString(Buffer, ArrayCount(Buffer), "Charges : %d", Ability->Cost.Charges);
						PushTextBillboard(RenderGroup, Font, FontHeight, At, Normal, Down, CardText, TextColor);
						At += FontHeight * Down;
					}
					if(Ability->Cost.Actions)
					{
						CardText = FormatString(Buffer, ArrayCount(Buffer), "Actions : %d", Ability->Cost.Actions);
						PushTextBillboard(RenderGroup, Font, FontHeight, At, Normal, Down, CardText, TextColor);
						At += FontHeight * Down;
					}
				} break;

				InvalidDefaultCase;
			}
		}
	}

#if 0
	u32 Rows = 5;
	u32 SpheresPerRow = 5;
	v3 Scale = V3(0.5f, 0.5f, 0.5f);
	f32 XGap = 1.1f;
	f32 YGap = 1.1f;
	v3 AtP = V3(-0.5f*SpheresPerRow*XGap, 0, 0);
	f32 XInit = AtP.x;
	for(u32 Row = 0;
		Row < Rows;
		++Row)
	{
		for(u32 Index = 0;
			Index < SpheresPerRow;
			++Index)
		{
			surface_material Material = {};
			Material.Roughness = ((f32)Index + 1)/(f32)(SpheresPerRow);
			Material.Metalness = ((f32)Row)/(f32)(Rows - 1);
			Material.Color = COLOR_COPPER;

			PushMesh(RenderGroup, AtP, Q(1,0,0,0), Scale,
				GetMeshID(GameState->Assets, Asset_Sphere),
				RenderGroup->WhiteTexture, Material);
			AtP.x += XGap;
		}

		AtP.x = XInit;
		AtP.y += YGap;
	}
#endif

	for(u32 DecalIndex = 0;
		DecalIndex < BattleMode->DecalCount;
		++DecalIndex)
	{
		oriented_box *DecalBox = BattleMode->Decals + DecalIndex;
		PushDecal(RenderGroup, DecalBox->P, DecalBox->Dim, DecalBox->Rotation, GetBitmapID(RenderGroup->Assets, Asset_Smile));
	}
}

internal void
UpdateAndRenderHUD(game_mode_battle *BattleMode, render_group *RenderGroup, ui_state *UI, v2i ScreenResolution, game_input *Input, v3 MouseRayDirection)
{
	BattleMode->ScreenCamera = DefaultScreenCamera(ScreenResolution);
	SetCamera(RenderGroup, &BattleMode->ScreenCamera);
	SetRenderFlags(RenderGroup, RenderFlag_NotLighted|RenderFlag_AlphaBlended|RenderFlag_TopMost|RenderFlag_SDF);

	font_id TimesFont = GetFontID(RenderGroup->Assets, Asset_TimesFont);

	v2 PassButtonDim = V2(200.0f, 130.0f);
	rectangle2 PassButton = Rectangle2(V2(ScreenResolution.x - PassButtonDim.x, 0),
									   V2((f32)ScreenResolution.x, PassButtonDim.y));
	ui_interaction PassInteraction = NullInteraction();
	PassInteraction.Type = UI_Interaction_PassTurn;
	b32 PassButtonSelected = ButtonInteractionSelected(UI, PassButton, PassInteraction, Input);

	f32 FontHeight = 50.0f;
	v4 TextColor = V4(1,1,0,1);
	v4 PassButtonColor = V4(0.4f, 0.1f, 0.4f, 0.5f);
	PassButtonColor.a = PassButtonSelected ? 0.75f : 0.5f;

	PushQuad(RenderGroup, PassButton.Min, PassButton.Max, PassButtonColor);
	PushText(RenderGroup, TimesFont, PassButton.Min + V2(0.1f*PassButtonDim.x, 0.5f*PassButtonDim.y),
		FontHeight, WrapZ("PASS"), TextColor);

	entity *HoveredEntity = PickEntityRayCast(BattleMode, BattleMode->WorldCamera.P, MouseRayDirection);
	if(HoveredEntity)
	{
		UI->HoveredID = HoveredEntity->ID;
	}
	else
	{
		UI->HoveredID = {};
	}

	BattleMode->HighlightID = {};
	FOR_EACH(Entity, BattleMode->EntityManager, entity)
	{
		ui_interaction SelectEntityInteraction = NullInteraction();
		SelectEntityInteraction.Type = UI_Interaction_SelectEntity;
		SelectEntityInteraction.EntityID = Entity->ID;
		b32 Hovered = (UI->HoveredID == Entity->ID);
		if(ButtonInteractionSelected(UI, Hovered, SelectEntityInteraction, Input))
		{
			BattleMode->HighlightID = Entity->ID;
		}
	}

	entity *SelectedEntity = GetEntity(BattleMode->EntityManager, UI->SelectedID);
	if(SelectedEntity)
	{
		if(SelectedEntity->AbilityCount)
		{
			v2 ScreenP = WorldCoordinatesToScreenCoordinates(&BattleMode->WorldCamera,
				SelectedEntity->P, ScreenResolution);
			v2i AbilityDim = V2i(200, 30);
			s32 Spacing = 5;

			ui_panel_positions AbilityPanelPositions = GetUIPanelPositions(AbilityDim, Spacing, SelectedEntity->AbilityCount, ScreenResolution, V2ToV2i(ScreenP));
			for(u32 AbilityIndex = 0;
				AbilityIndex < SelectedEntity->AbilityCount;
				++AbilityIndex)
			{
				ability *Ability = SelectedEntity->Abilities + AbilityIndex;

				rectangle2 AbilityButton = Rectangle2(V2iToV2(AbilityPanelPositions.At),
					V2iToV2(AbilityPanelPositions.At + AbilityPanelPositions.Dim));
				f32 Alpha = 0.5f;

				ui_interaction SelectAbilityInteraction = NullInteraction();
				SelectAbilityInteraction.Type = UI_Interaction_SelectAbility;
				SelectAbilityInteraction.EntityID = UI->SelectedID;
				SelectAbilityInteraction.AbilityIndex = AbilityIndex;
				b32 AbilityButtonSelected = ButtonInteractionSelected(UI, AbilityButton, SelectAbilityInteraction, Input);
				if(AbilityButtonSelected)
				{
					UI->HoveredID = UI->SelectedID;
					HoveredEntity = SelectedEntity;
					UI->HoveredAbilityIndex = AbilityIndex;

					Alpha = 0.75f;
				}

				if(AbilityIndex == UI->SelectedAbilityIndex)
				{
					Alpha = 1.0f;
				}

				RenderAbilityIcon(UI, Ability, AbilityPanelPositions.At,
					AbilityPanelPositions.At + AbilityPanelPositions.Dim, Alpha);
				AbilityPanelPositions.At = AbilityPanelPositions.At + AbilityPanelPositions.Advance;
			}
		}
	}

	if(Input->KeyStates[Key_RightClick].WasReleased ||
	   Input->KeyStates[Key_Esc].WasPressed)
	{
		UI->SelectedID = {0};
		UI->SelectedAbilityIndex = U32Maximum;
	}

	if(Input->KeyStates[Key_Q].Pressed)
	{
		if(HoveredEntity)
		{
			if(HoveredEntity->Flags & EntityFlag_InHand)
			{
				UI->HoveredAbilityIndex = 0;
			}

			if(UI->HoveredAbilityIndex < U32Maximum)
			{
				ability *Ability = HoveredEntity->Abilities + UI->HoveredAbilityIndex;
				v2i AbilityInfoDim = V2i(400, 300);
				ui_panel_positions AbilityInfoPanelPositions = GetUIPanelPositions(AbilityInfoDim, 0, 1, ScreenResolution, V2ToV2i(Input->MouseP));
				RenderAbilityInfo(UI, Ability,
					AbilityInfoPanelPositions.At,
					AbilityInfoPanelPositions.At + AbilityInfoPanelPositions.Dim);
			}
			else if(HoveredEntity->Flags & EntityFlag_OnBoard)
			{
				entity_list InfoEntityList = GetEntitiesAtPosition(&BattleMode->Board, HoveredEntity->BoardPosition);
				v2i InfoDim = V2i(200, 300);
				s32 InfoSpacing = 25;

				ui_panel_positions InfoPanelPositions = GetUIPanelPositions(InfoDim, InfoSpacing, InfoEntityList.EntityCount, ScreenResolution, V2ToV2i(Input->MouseP));
				for(u32 EntityIndex = 0;
					EntityIndex < InfoEntityList.EntityCount;
					++EntityIndex)
				{
					entity_id ID = InfoEntityList.EntityIDs[EntityIndex];
					entity *Entity = GetEntity(BattleMode->EntityManager, ID);
					RenderEntityInfo(UI, Entity,
						InfoPanelPositions.At,
						InfoPanelPositions.At + InfoPanelPositions.Dim);
					InfoPanelPositions.At = InfoPanelPositions.At + InfoPanelPositions.Advance;
				}
			}

		}
	}

	f32 LayoutHeight = 25.0f;
	v2 LayoutAt = V2(25.0f, 5.5f*LayoutHeight);
	rectangle2 LayoutRect = Rectangle2(LayoutAt, LayoutAt);
	ui_layout Layout = BeginUILayout(UI, Asset_TimesFont, LayoutHeight, V4(1,1,1,1), LayoutRect);

	entity *Player1 = GetEntity(BattleMode->EntityManager, BattleMode->Players[Player_1]);
	entity *Player2 = GetEntity(BattleMode->EntityManager, BattleMode->Players[Player_2]);

	BeginRow(&Layout);
	Layout.Color = Player1->Color;
	Labelf(&Layout, "%s : G:%d VP:%d",
		WrapZ(Names_player_name[Player1->Owner]),
		Player1->Gold,
		Player1->VictoryPoints);
	EndRow(&Layout);

	BeginRow(&Layout);
	Layout.Color = Player2->Color;
	Labelf(&Layout, "%s : G:%d VP:%d",
		WrapZ(Names_player_name[Player2->Owner]),
		Player2->Gold,
		Player2->VictoryPoints);
	EndRow(&Layout);

	BeginRow(&Layout);
	Layout.Color = V4(1,1,0,1);
	Labelf(&Layout, "Current Player : %s", WrapZ(Names_player_name[BattleMode->CurrentPlayer]));
	EndRow(&Layout);

	entity *Player = GetEntity(BattleMode->EntityManager, BattleMode->Players[BattleMode->CurrentPlayer]);
	if(Player)
	{
		BeginRow(&Layout);
		Labelf(&Layout, "Actions remaining : %d/%d",
			Player->ActionsRemaining,
			BattleMode->ActionsPerTurn);
		EndRow(&Layout);
	}

	BeginRow(&Layout);
	Labelf(&Layout, "Turn %d, Phase %s",
		BattleMode->CurrentTurn,
		WrapZ(Names_game_phase[BattleMode->Phase]));
	EndRow(&Layout);
	EndUILayout(&Layout);

	if(BattleMode->Phase == Phase_GameOver)
	{
		b32 Player1Win = (Player1->VictoryPoints >= BattleMode->PointsToWin);
		b32 Player2Win = (Player2->VictoryPoints >= BattleMode->PointsToWin);
		string EndGameString = {};
		if(Player1Win && Player2Win)
		{
			EndGameString = WrapZ("Draw!");
		}
		else if(Player1Win)
		{
			EndGameString = WrapZ("Player 1 Wins!");
		}
		else if(Player2Win)
		{
			EndGameString = WrapZ("Player 2 Wins!");
		}

		f32 TextHeight = 50.0f;
		v2 TextDim = GetTextDim(RenderGroup, TimesFont, TextHeight, EndGameString);
		PushText(RenderGroup, TimesFont, 0.5f*ScreenResolution, TextHeight, EndGameString);
	}

#if 0
	// NOTE: Font texture test.
	renderer_texture LibmonoFont = GetFontBitmap(GameState->Assets, GetFontID(GameState->Assets, Asset_LiberationMonoFont));
	renderer_texture TimesFont = GetFontBitmap(GameState->Assets, GetFontID(GameState->Assets, Asset_TimesFont));
	if(LibmonoFont.Index && TimesFont.Index)
	{
		PushSetup(RenderGroup, &BattleMode->ScreenCamera, Target_Overlay, ScreenResolution, RenderFlag_NotLighted|RenderFlag_AlphaBlended|RenderFlag_TopMost);

		PushTexture(RenderGroup, V2(400, 400), V2(600, 0), V2(0, 600), LibmonoFont);
		PushTexture(RenderGroup, V2(1100, 400), V2(600, 0), V2(0, 600), TimesFont);
	}
#endif
}

internal void
UpdateAndRenderBattleMode(game_state *GameState, game_mode_battle *BattleMode,
	game_render_commands *RenderCommands, game_input *Input)
{
	BeginBoardLists(BattleMode->EntityManager, &BattleMode->Board);
	ui_state *UI = &BattleMode->UI;
	UI->HoveredAbilityIndex = U32Maximum;

	v2i ScreenResolution = Input->ScreenResolution;
	camera *WorldCamera = &BattleMode->WorldCamera;
	f32 WorldNear = 0.01f;
	f32 WorldFar = 50.0f;
	f32 AspectRatio = (f32)ScreenResolution.x/(f32)ScreenResolution.y;
	f32 WorldHorizontalFOV = 90.0f*(Pi32/180.0f);
	if(ScreenResolution.x < ScreenResolution.y)
	{
		WorldHorizontalFOV *= AspectRatio;
	}

	ChangeCameraSettings(WorldCamera, WorldHorizontalFOV, AspectRatio,
		WorldNear, WorldFar);

	v3 Up = V3(0,1,0);
	v3 Forward = V3(0,0,-1);
	v3 Right = Cross(Forward, Up);
	v3 CameraFacingDirection = Forward;
	f32 CameraDistance = 10.0f;
	if(GameState->UseDebugCamera)
	{
		if(Input->KeyStates[Key_RightClick].Pressed)
		{
			f32 Sensitivity = 0.005f;
			BattleMode->Pitch += Sensitivity*Input->dMouseP.y;
			BattleMode->Pitch = Clamp(BattleMode->Pitch, -0.5f*Pi32, 0.5f*Pi32);
			BattleMode->Yaw += -Sensitivity*Input->dMouseP.x;
		}

		quaternion PitchQ = RotationQuaternion(Right, BattleMode->Pitch);
		quaternion YawQ = RotationQuaternion(Up, BattleMode->Yaw);
		quaternion NewCameraRotation = NormalizeIfNeeded(YawQ*PitchQ);
		CameraFacingDirection = RotateBy(Forward, NewCameraRotation);
		v3 CameraRight = RotateBy(Right, YawQ);

		v3 WorldCameraMove = {};
		r32 CameraMoveScale = 15.0f*Input->Simdt;
		if(Input->KeyStates[Key_Up].Pressed)
		{
			WorldCameraMove += CameraMoveScale*CameraFacingDirection;
		}
		if(Input->KeyStates[Key_Down].Pressed)
		{
			WorldCameraMove += -CameraMoveScale*CameraFacingDirection;
		}
		if(Input->KeyStates[Key_Left].Pressed)
		{
			WorldCameraMove += -CameraMoveScale*CameraRight;
		}
		if(Input->KeyStates[Key_Right].Pressed)
		{
			WorldCameraMove += CameraMoveScale*CameraRight;
		}
		if(Input->KeyStates[Key_Space].Pressed)
		{
			WorldCameraMove += CameraMoveScale*Up;
		}
		if(Input->KeyStates[Key_Ctrl].Pressed)
		{
			WorldCameraMove += -CameraMoveScale*Up;
		}

		v3 NewCameraP = {};
		NewCameraP = WorldCamera->P + WorldCameraMove;
		MoveCamera(WorldCamera, NewCameraP);
		RotateCamera(WorldCamera, NewCameraRotation);
	}
	else
	{
		CameraFacingDirection = Normalize(V3(0.0f, -1.5f, -1.0f));
		v3 CameraLookAt = V3(0.0f, 0.0f, 2.5f);
		v3 NewCameraP = CameraLookAt + -CameraDistance*CameraFacingDirection;
		v3 Z = -CameraFacingDirection;
		v3 X = NormalizeOr(Cross(Up, Z), Right);
		v3 Y = Cross(Z, X);
		quaternion NewCameraRotation = RotationQuaternion(X, Y, Z);

		MoveCamera(WorldCamera, NewCameraP);
		RotateCamera(WorldCamera, NewCameraRotation);
	}

	v3 MouseRayDirection = ScreenPToWorldDirection(WorldCamera, Input->MouseP, ScreenResolution);

	switch(BattleMode->Phase)
	{
		case Phase_GameBegin:
		{
			SetupPlayers(GameState, BattleMode);
			SetupBoard(GameState, BattleMode, &BattleMode->Board);

			AddCard(GameState, BattleMode, Player_1, Ability_BuildHeadquarters);
			AddCard(GameState, BattleMode, Player_2, Ability_BuildHeadquarters);

			BattleMode->CurrentPlayer = Player_1;
			ChangeGamePhase(BattleMode, UI, Phase_PlaceHQ);
			entity *Player = GetEntity(BattleMode->EntityManager, BattleMode->Players[BattleMode->CurrentPlayer]);
			Player->ActionsRemaining = 1;
		} break;

		case Phase_HQEnd:
		{
			if(BattleMode->CurrentPlayer == Player_1)
			{
				BattleMode->CurrentPlayer = Player_2;
				ChangeGamePhase(BattleMode, UI, Phase_PlaceHQ);
				entity *Player = GetEntity(BattleMode->EntityManager, BattleMode->Players[BattleMode->CurrentPlayer]);
				Player->ActionsRemaining = 1;
			}
			else
			{
				entity *Player = GetEntity(BattleMode->EntityManager, BattleMode->Players[BattleMode->CurrentPlayer]);
				entity *Opponent = GetOpponent(BattleMode, Player);

				u32 StartingHandSize = 5;
				for(u32 Count = 0;
					Count < StartingHandSize;
					++Count)
				{
					DrawRandomCard(GameState, BattleMode, Player);
					DrawRandomCard(GameState, BattleMode, Opponent);
				}

				ChangeGamePhase(BattleMode, UI, Phase_TurnStart);
			}
		} break;

		case Phase_PlaceHQ:
		{

		} break;

		case Phase_TurnStart:
		{
			entity *Player = GetEntity(BattleMode->EntityManager, BattleMode->Players[BattleMode->CurrentPlayer]);
			FOR_EACH(Entity, BattleMode->EntityManager, entity)
			{
				if(Entity->Owner == BattleMode->CurrentPlayer)
				{
					if(Entity->Flags & EntityFlag_IsBuilding)
					{
						Entity->Defense = Entity->MaxDefense;
					}

					for(u32 AbilityIndex = 0;
						AbilityIndex < Entity->AbilityCount;
						++AbilityIndex)
					{
						ability *Ability = Entity->Abilities + AbilityIndex;
						++Ability->CurrentCooldown;

						if(Ability->Type == AbilityType_Reaction)
						{
							if(Ability->Trigger == Reaction_TurnStart)
							{
								UseAbility(GameState, BattleMode, Player, Entity, Ability, UI->TargetIDs);
							}
						}
					}
				}
			}

			ChangeGamePhase(BattleMode, UI, Phase_PlayerActions);
			Player->ActionsRemaining = BattleMode->ActionsPerTurn;
		} break;

		case Phase_TurnEnd:
		{
			entity *Player = GetEntity(BattleMode->EntityManager, BattleMode->Players[BattleMode->CurrentPlayer]);
			entity *Opponent = GetOpponent(BattleMode, Player);
			if((Player->VictoryPoints >= BattleMode->PointsToWin) ||
			   (Opponent->VictoryPoints >= BattleMode->PointsToWin))
			{
				ChangeGamePhase(BattleMode, UI, Phase_GameOver);
				BattleMode->CurrentPlayer = Player_None;
			}
			else
			{
				if(BattleMode->CurrentPlayer == Player_1)
				{
					BattleMode->CurrentPlayer = Player_2;
					++BattleMode->CurrentTurn;
				}
				else
				{
					BattleMode->CurrentPlayer = Player_1;
				}

				ChangeGamePhase(BattleMode, UI, Phase_TurnStart);
			}
		} break;

		case Phase_PlayerActions:
		{

		} break;

		case Phase_GameOver:
		{

		} break;

		InvalidDefaultCase;
	}

	if(Input->KeyStates[Key_K].WasPressed)
	{
		AddPhysicsBox(GameState, BattleMode, V3(0,6,0), V3(1,1,1));
	}

	render_group OverlayGroup = BeginRender(GameState->Assets, RenderCommands, Target_Overlay);
	BeginFrameUI(UI, &OverlayGroup, Input);
	UpdateAndRenderHUD(BattleMode, &OverlayGroup, UI, ScreenResolution, Input, MouseRayDirection);
	EndRender(&OverlayGroup);
	EndBattleModeUI(GameState, UI, BattleMode, Input);

	v4 SceneClearColor = {0.1f, 0.2f, 0.2f, 1.0f};
	render_group SceneGroup = BeginRender(GameState->Assets, RenderCommands, Target_Scene, SceneClearColor);
	UpdateAndRenderEntities(GameState, BattleMode, &SceneGroup, ScreenResolution, Input->Simdt);
	UpdateParticleSystem(GameState, BattleMode->ParticleSystem, Input->Simdt);
	RenderParticleSystem(GameState, BattleMode->ParticleSystem, &SceneGroup, WorldCamera->P);
	EndRender(&SceneGroup);

	EndBoardLists(&BattleMode->Board);

	if(BattleMode->GameOver_ || Input->KeyStates[Key_M].WasPressed)
	{
		PlayTitleScreen(GameState);
	}
}
