/*@H
* File: cardcade_editor.cpp
* Author: Jesse Calvert
* Created: December 6, 2018, 16:37
* Last modified: December 17, 2018, 18:56
*/

internal void
DeleteAbility(entity_info *EntityInfo, ability_name *ToDelete, u32 AbilityIndexToDelete)
{
	ability_name *PrevAbility = ToDelete;
	for(u32 Index = AbilityIndexToDelete + 1;
		Index < EntityInfo->AbilityCount;
		++Index)
	{
		ability_name *Ability = EntityInfo->Abilities + Index;
		*PrevAbility = *Ability;
		PrevAbility = Ability;
	}

	--EntityInfo->AbilityCount;
}

internal void
DeleteVisiblePiece(entity_info *EntityInfo, entity_visible_piece *ToDelete, u32 PieceIndexToDelete)
{
	entity_visible_piece *PrevPiece = ToDelete;
	for(u32 Index = PieceIndexToDelete + 1;
		Index < EntityInfo->PieceCount;
		++Index)
	{
		entity_visible_piece *Piece = EntityInfo->Pieces + Index;
		*PrevPiece = *Piece;
		PrevPiece = Piece;
	}

	--EntityInfo->PieceCount;
}

internal void
ApplyEdit(game_editor *Editor, editor_edit *Edit, edit_change_type ChangeType)
{
	asset *Asset = 0;

	switch(Edit->Type)
	{
		case Edit_EntityInfo:
		{
			entity_info_edit *EntityInfoEdit = &Edit->EntityInfoEdit;
			*EntityInfoEdit->Ptr = EntityInfoEdit->Edit[ChangeType];

			Asset = GetEntityInfoAsset(Editor->Assets, EntityInfoEdit->Ptr);
		} break;
	}

	if(Asset)
	{
		Asset->IsDirty = true;
	}
}

internal void
Undo(game_editor *Editor)
{
	if(Editor->LastEdit != &Editor->EditSentinel)
	{
		ApplyEdit(Editor, Editor->LastEdit, EditChange_From);
		Editor->LastEdit = Editor->LastEdit->Prev;
	}
}

internal void
Redo(game_editor *Editor)
{
	if(Editor->LastEdit->Next != &Editor->EditSentinel)
	{
		ApplyEdit(Editor, Editor->LastEdit->Next, EditChange_To);
		Editor->LastEdit = Editor->LastEdit->Next;
	}
}

internal void
Revert(game_editor *Editor)
{
	while(Editor->LastEdit != &Editor->EditSentinel)
	{
		Undo(Editor);
	}
}

internal editor_edit *
GetOrCreateEdit(game_editor *Editor)
{
	editor_edit *Result = Editor->EditFreeList;
	if(Result)
	{
		Editor->EditFreeList = Result->Next;
		ZeroStruct(*Result);
	}
	else
	{
		Result = PushStruct(&Editor->Region, editor_edit);
	}

	return Result;
}

internal b32
EditEntityInfo(game_editor *Editor, entity_info *Info,
	entity_info OldInfo, entity_info NewInfo)
{
	b32 ChangeMade = false;
	if(!MemoryCompare(&OldInfo, &NewInfo, sizeof(entity_info)))
	{
		editor_edit *Edit = GetOrCreateEdit(Editor);
		Edit->Type = Edit_EntityInfo;
		entity_info_edit *InfoEdit = &Edit->EntityInfoEdit;
		InfoEdit->Ptr = Info;
		InfoEdit->Edit[EditChange_From] = OldInfo;
		InfoEdit->Edit[EditChange_To] = NewInfo;

		DLIST_INSERT(Editor->LastEdit, Edit);
		Editor->LastEdit = Editor->LastEdit->Next;

		ApplyEdit(Editor, Edit, EditChange_To);

		if(Editor->LastEdit->Next != &Editor->EditSentinel)
		{
			Editor->EditSentinel.Prev->Next = Editor->EditFreeList;
			Editor->EditFreeList = Editor->LastEdit->Next;

			Editor->LastEdit->Next = &Editor->EditSentinel;
			Editor->EditSentinel.Prev = Editor->LastEdit;
		}

		ChangeMade = true;
	}

	return ChangeMade;
}

internal game_editor *
CreateEditor(assets *Assets)
{
	game_editor *Editor = BootstrapPushStruct(game_editor, Region);
	Editor->Assets = Assets;

	DLIST_INIT(&Editor->EditSentinel);
	Editor->LastEdit = &Editor->EditSentinel;
	Editor->LastSaveEdit = &Editor->EditSentinel;

	return Editor;
}

internal void
UpdateAndRenderEditor(game_state *GameState, game_editor *Editor, ui_state *UI,
	game_mode_battle *BattleMode, game_input *Input)
{
	if(!Editor)
	{
		Editor = GameState->Editor = CreateEditor(GameState->Assets);
	}

	assets *Assets = Editor->Assets;
	memory_region *FrameRegion = &GameState->FrameRegion;
	render_group *Group = UI->Group;
	v2i ScreenResolution = Input->ScreenResolution;
	rectangle2 EditorRect = Rectangle2(V2(0,0), V2(0.5f*ScreenResolution.x, 0.9f*ScreenResolution.y));

	SetRenderFlags(Group, RenderFlag_TopMost|RenderFlag_AlphaBlended);
	v4 BackgroundColor = V4(0,0,0,0.5f);
	PushQuad(Group, EditorRect.Min, EditorRect.Max, BackgroundColor);

	TemporaryClipRectChange(Group, V2ToV2i(EditorRect.Min), V2ToV2i(EditorRect.Max));
	SetRenderFlags(Group, RenderFlag_SDF|RenderFlag_TopMost|RenderFlag_AlphaBlended);

	ui_layout Layout = BeginUILayout(UI, Asset_LiberationMonoFont, 20.0f, V4(1,1,1,1), EditorRect);

	v3 MouseRayDirection = ScreenPToWorldDirection(&BattleMode->WorldCamera, Input->MouseP, ScreenResolution);
	entity *HoveredEntity = PickEntityRayCast(BattleMode, BattleMode->WorldCamera.P, MouseRayDirection);
	if(HoveredEntity)
	{
		UI->HoveredID = HoveredEntity->ID;
		if(Input->KeyStates[Key_LeftClick].WasPressed)
		{
			UI->SelectedID = UI->HoveredID;
		}
	}
	else
	{
		UI->HoveredID = {};
	}

	BeginBoardLists(BattleMode->EntityManager, &BattleMode->Board);
	entity *SelectedEntity = GetEntity(BattleMode->EntityManager, UI->SelectedID);
	if(SelectedEntity)
	{
		entity_list EntityList = {};
		if(SelectedEntity->Flags & EntityFlag_OnBoard)
		{
			EntityList = GetEntitiesAtPosition(&BattleMode->Board, SelectedEntity->BoardPosition);
		}
		else
		{
			EntityList.EntityIDs[EntityList.EntityCount++] = UI->SelectedID;
		}

		BeginRow(&Layout);
		b32 ChangesMade = (Editor->LastSaveEdit != Editor->LastEdit);
		if(Button(&Layout, ConstZ("SAVE ALL"), UIIDFromPointer(Editor), ChangesMade))
		{
			WriteDirtyAssets(Assets, FrameRegion);
			Editor->LastSaveEdit = Editor->LastEdit;
		}
		b32 CanUndo = (Editor->LastEdit != &Editor->EditSentinel);
		if(Button(&Layout, ConstZ("REVERT"), UIIDFromPointer(Editor), CanUndo))
		{
			Revert(Editor);
		}
		EndRow(&Layout);

		BeginRow(&Layout);
		if(Button(&Layout, ConstZ("UNDO"), UIIDFromPointer(Editor), CanUndo))
		{
			Undo(Editor);
		}
		if(Button(&Layout, ConstZ("REDO"), UIIDFromPointer(Editor), (Editor->LastEdit->Next != &Editor->EditSentinel)))
		{
			Redo(Editor);
		}
		EndRow(&Layout);

		BeginRow(&Layout);
		Label(&Layout, ConstZ("Entity Select"));
		EndRow(&Layout);

		entity *EditingEntity = 0;
		entity_info *EntityInfo = 0;
		for(u32 EntityIndex = 0;
			EntityIndex < EntityList.EntityCount;
			++EntityIndex)
		{
			entity_id ID = EntityList.EntityIDs[EntityIndex];
			entity *Entity = GetEntity(BattleMode->EntityManager, ID);
			BeginRow(&Layout);
			if(ID == Editor->EditingEntity)
			{
				Label(&Layout, ConstZ("-->"));
				EntityInfo = Entity->Info;
				EditingEntity = Entity;
			}
			if(Button(&Layout, Entity->Info->DisplayName, UIIDFromPointer(Entity), true))
			{
				Editor->EditingEntity = ID;
			}
			EndRow(&Layout);
		}

		if(EntityInfo)
		{
			entity_info OldInfo = *EntityInfo;

			BeginRow(&Layout);
			Labelf(&Layout, "Entity Info Editor (%s)", WrapZ(Names_entity_type[EntityInfo->Type]));
			EndRow(&Layout);

			BeginRow(&Layout);
			Label(&Layout, ConstZ("Abilities"));
			if(Button(&Layout, ConstZ("ADD"), UIIDFromPointer(EntityInfo),
				(EntityInfo->AbilityCount < ArrayCount(EntityInfo->Abilities))))
			{
				u32 AbilityIndex = EntityInfo->AbilityCount++;
				ability_name *NewAbility = EntityInfo->Abilities + AbilityIndex;
				ZeroStruct(*NewAbility);
			}
			EndRow(&Layout);
			BeginIndent(&Layout);
			for(u32 AbilityIndex = 0;
				AbilityIndex < EntityInfo->AbilityCount;
				++AbilityIndex)
			{
				ability_name *Ability = EntityInfo->Abilities + AbilityIndex;
				string Name = WrapZ(Names_ability_name[*Ability]);

				BeginRow(&Layout);
				if(Button(&Layout, ConstZ("DEL"), UIIDFromPointer(Ability), true))
				{
					DeleteAbility(EntityInfo, Ability, AbilityIndex);
				}

				EditableType(&Layout, Name, UIIDFromPointer(Ability), (u32 *)Ability, Ability_Count);
				EndRow(&Layout);
			}
			EndIndent(&Layout);

			BeginRow(&Layout);
			Label(&Layout, ConstZ("Pieces"));
			if(Button(&Layout, ConstZ("ADD"), UIIDFromPointer(EntityInfo),
				(EntityInfo->PieceCount < ArrayCount(EntityInfo->Pieces))))
			{
				u32 PieceIndex = EntityInfo->PieceCount++;
				entity_visible_piece *Piece = EntityInfo->Pieces + PieceIndex;
				ZeroStruct(*Piece);
			}
			EndRow(&Layout);
			BeginIndent(&Layout);
			for(u32 PieceIndex = 0;
				PieceIndex < EntityInfo->PieceCount;
				++PieceIndex)
			{
				entity_visible_piece *Piece = EntityInfo->Pieces + PieceIndex;
				string Name = WrapZ(Names_entity_visible_piece_type[Piece->Type]);

				BeginIndent(&Layout);
				BeginRow(&Layout);

				if(Button(&Layout, ConstZ("DEL"), UIIDFromPointer(Piece), true))
				{
					DeleteVisiblePiece(EntityInfo, Piece, PieceIndex);
				}
				EditableType(&Layout, Name, UIIDFromPointer(Piece), (u32 *)&Piece->Type, PieceType_Count);
				Labelf(&Layout, " BitmapID: %d MeshID: %d", Piece->BitmapID.Value, Piece->MeshID.Value);
				EndRow(&Layout);

				BeginIndent(&Layout);
				BeginRow(&Layout);
				Labelf(&Layout, "Dim:");
				EditableV3(&Layout, UIIDFromPointer(Piece), &Piece->Dim, 0.0f, 10.0f);
				EndRow(&Layout);

				BeginRow(&Layout);
				Labelf(&Layout, "Rotation:");
				EditableQuaternion(&Layout, UIIDFromPointer(Piece), &Piece->Rotation);
				EndRow(&Layout);

				BeginRow(&Layout);
				Labelf(&Layout, "Offset:");
				EditableV3(&Layout, UIIDFromPointer(Piece), &Piece->Offset, -10.0f, 10.0f);
				EndRow(&Layout);

				BeginRow(&Layout);
				Labelf(&Layout, "Roughness:");
				EditableF32(&Layout, UIIDFromPointer(Piece), &Piece->Material.Roughness, 0.0f, 1.0f);

				Labelf(&Layout, "Metalness:");
				EditableF32(&Layout, UIIDFromPointer(Piece), &Piece->Material.Metalness, 0.0f, 1.0f);
				EndRow(&Layout);

				BeginRow(&Layout);
				Labelf(&Layout, "Color:");
				EditableV4(&Layout, UIIDFromPointer(Piece), &Piece->Material.Color, 0.0f, 1.0f);
				EndRow(&Layout);

				EndIndent(&Layout);
				EndRow(&Layout);
				EndIndent(&Layout);
			}
			EndIndent(&Layout);

			BeginRow(&Layout);
			EditableS32(&Layout, ConstZ("Attack"), UIIDFromPointer(EntityInfo), &EntityInfo->Attack);
			EndRow(&Layout);

			BeginRow(&Layout);
			EditableS32(&Layout, ConstZ("MaxDefense"), UIIDFromPointer(EntityInfo), &EntityInfo->MaxDefense);
			EndRow(&Layout);

			BeginRow(&Layout);
			EditableS32(&Layout, ConstZ("Influence"), UIIDFromPointer(EntityInfo), &EntityInfo->Influence);
			EndRow(&Layout);

			BeginRow(&Layout);
			EditableS32(&Layout, ConstZ("VictoryPoints"), UIIDFromPointer(EntityInfo), &EntityInfo->VictoryPoints);
			EndRow(&Layout);

			BeginRow(&Layout);
			Labelf(&Layout, "Color : (");
			EditableV4(&Layout, UIIDFromPointer(EntityInfo), &EntityInfo->Color, 0.0f, 1.0f);
			Labelf(&Layout, ")");
			EndRow(&Layout);

			BeginRow(&Layout);
			Labelf(&Layout, "Flags:");
			BeginIndent(&Layout);
			if(ButtonToggle(&Layout, ConstZ("Traversable"), UIIDFromPointer(EntityInfo),
				EntityInfo->Flags & EntityFlag_Traversable))
			{
				EntityInfo->Flags ^= EntityFlag_Traversable;
			}
			if(ButtonToggle(&Layout, ConstZ("OnBoard"), UIIDFromPointer(EntityInfo),
				EntityInfo->Flags & EntityFlag_OnBoard))
			{
				EntityInfo->Flags ^= EntityFlag_OnBoard;
			}
			if(ButtonToggle(&Layout, ConstZ("InHand"), UIIDFromPointer(EntityInfo),
				EntityInfo->Flags & EntityFlag_InHand))
			{
				EntityInfo->Flags ^= EntityFlag_InHand;
			}
			if(ButtonToggle(&Layout, ConstZ("IsBuilding"), UIIDFromPointer(EntityInfo),
				EntityInfo->Flags & EntityFlag_IsBuilding))
			{
				EntityInfo->Flags ^= EntityFlag_IsBuilding;
			}
			if(ButtonToggle(&Layout, ConstZ("IsUnit"), UIIDFromPointer(EntityInfo),
				EntityInfo->Flags & EntityFlag_IsUnit))
			{
				EntityInfo->Flags ^= EntityFlag_IsUnit;
			}

			EndIndent(&Layout);
			EndRow(&Layout);

			entity_info NewInfo = *EntityInfo;
			EditEntityInfo(Editor, EntityInfo, OldInfo, NewInfo);

			FOR_EACH(Entity, BattleMode->EntityManager, entity)
			{
				if(Entity->Info == EntityInfo)
				{
					EntityRevertToInfo(BattleMode, Entity);
				}
			}
		}
	}
	EndBoardLists(&BattleMode->Board);

	EndUILayout(&Layout);
}
