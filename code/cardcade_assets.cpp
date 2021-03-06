/*@H
* File: cardcade_assets.cpp
* Author: Jesse Calvert
* Created: January 15, 2018, 20:01
* Last modified: December 17, 2018, 18:54
*/

internal f32
GetFontLineHeight(font_sla_v3 *FontInfo)
{
	f32 Result = 0.0f;
	Assert(FontInfo);
	{
		Result = (FontInfo->Ascent - FontInfo->Descent + FontInfo->Linegap);
	}

	return Result;
}

internal f32
GetFontTotalLineHeight(font_sla_v3 *FontInfo, f32 Scale)
{
	f32 Result = 0.0f;
	Assert(FontInfo);
	{
		f32 DescentPixels = (FontInfo->Ascent - FontInfo->Descent + FontInfo->Linegap);
		Result = Scale*DescentPixels;
	}
	return Result;
}

internal f32
GetFontScale(render_group *Group, font_id Font, f32 Height)
{
	f32 Result = 0.0f;
	font_sla_v3 *FontInfo = GetFontInfo(Group->Assets, Font);
	Assert(FontInfo);
	{
		Result = Height / GetFontLineHeight(FontInfo);
	}
	return Result;
}

internal f32
GetFontDescent(render_group *Group, font_id Font, f32 Height)
{
	f32 Result = 0.0f;
	font_sla_v3 *FontInfo = GetFontInfo(Group->Assets, Font);
	Assert(FontInfo);
	{
		f32 Scale = GetFontScale(Group, Font, Height);
		Result = -Scale*FontInfo->Descent;
	}
	return Result;
}

internal f32
GetFontAscent(render_group *Group, font_id Font, f32 Height)
{
	f32 Result = 0.0f;
	font_sla_v3 *FontInfo = GetFontInfo(Group->Assets, Font);
	Assert(FontInfo);
	{
		f32 Scale = GetFontScale(Group, Font, Height);
		Result = Scale*FontInfo->Ascent;
	}
	return Result;
}

internal f32
GetFontLineGap(render_group *Group, font_id Font, f32 Height)
{
	f32 Result = 0.0f;
	font_sla_v3 *FontInfo = GetFontInfo(Group->Assets, Font);
	Assert(FontInfo);
	{
		f32 Scale = GetFontScale(Group, Font, Height);
		Result = Scale*FontInfo->Linegap;
	}
	return Result;
}

internal asset_memory_header *
MergeBlockIfPossible(asset_memory_header *Header)
{
	Assert(!Header->InUse);

	asset_memory_header *Result = Header;

	if(!Header->Prev->InUse)
	{
		Result = Header->Prev;
		Result->BlockSize += Header->BlockSize + sizeof(asset_memory_header);

		DLIST_REMOVE(Header);
	}

	if(!Result->Next->InUse)
	{
		Result->BlockSize += Result->Next->BlockSize + sizeof(asset_memory_header);

		asset_memory_header *ToRemove = Result->Next;
		DLIST_REMOVE(ToRemove);
	}

	return Result;
}

internal void
AllocateBlock(asset_memory_header *Header, u32 Size)
{
	Header->InUse = true;

	u32 ExtraSpace = Header->BlockSize - Size;
	// TODO: Should this depend on the smallest size of assets?
	u32 MinimumExtraSpace = 4096;
	if(ExtraSpace > MinimumExtraSpace)
	{
		Header->BlockSize = Header->BlockSize - ExtraSpace;
		asset_memory_header *NewBlock = (asset_memory_header *) (((u8 *)(Header + 1)) + Header->BlockSize);
		DLIST_INSERT(Header, NewBlock);
		NewBlock->InUse = false;
		NewBlock->BlockSize = ExtraSpace - sizeof(asset_memory_header);

		MergeBlockIfPossible(NewBlock);
	}
}

internal void
UnloadAsset(assets *Assets, asset *Asset)
{
	asset_memory_header *Header = Asset->MemoryHeader;

	Asset->State = Asset_Unloaded;
	if(Asset->Next && Asset->Prev)
	{
		DLIST_REMOVE(Asset);
	}

	if(Header)
	{
		Header->InUse = false;
		MergeBlockIfPossible(Header);
		Asset->MemoryHeader = 0;
	}

#if GAME_DEBUG
	Outf(Assets->Info, "Asset unloaded (%s)", WrapZ(Names_asset_name[Asset->Name]));
#endif
}

internal asset_memory_header *
GetAssetMemoryBlock(assets *Assets, u32 Size)
{
	TIMED_FUNCTION();

	asset_memory_header *Result = 0;

	BeginTicketMutex(&Assets->AssetMemoryMutex);

	while(!Result)
	{
		for(asset_memory_header *Header = Assets->MemorySentinel.Next;
			Header != &Assets->MemorySentinel;
			Header = Header->Next)
		{
			if(!Header->InUse && (Header->BlockSize >= Size))
			{
				AllocateBlock(Header, Size);
				Result = Header;
				break;
			}
		}

		if(!Result)
		{
			for(asset *OldAsset = Assets->LoadedAssetsSentinel.Prev;
				OldAsset != &Assets->LoadedAssetsSentinel;
				OldAsset = OldAsset->Prev)
			{
				if(OldAsset->State == Asset_Loaded)
				{
					UnloadAsset(Assets, OldAsset);
					break;
				}
			}
		}
	}

	EndTicketMutex(&Assets->AssetMemoryMutex);

	return Result;
}

struct load_asset_data
{
	task_memory *TaskMemory;

	asset *Asset;
	platform_file *AssetFile;
	u32 Offset;
	u32 Size;
	u8 *Dest;

	// NOTE: For mesh loading:
	u32 VerticesSize;

	render_memory_op_queue *RenderOpQueue;
};
internal void
LoadAssetImmediate(load_asset_data *WorkData)
{
	TIMED_FUNCTION();
	asset *Asset = WorkData->Asset;

	Platform->ReadFromFile((u8 *)WorkData->Dest, WorkData->AssetFile,
			WorkData->Offset, WorkData->Size);

	switch(Asset->Type)
	{
		case AssetType_Bitmap:
		{
			render_memory_op Op = {};
			Op.Type = RenderOp_LoadTexture;
			load_texture_op *LoadTexture = &Op.LoadTexture;
			LoadTexture->Texture = Asset->Bitmap;
			LoadTexture->Pixels = WorkData->Dest;
			AddRenderOp(WorkData->RenderOpQueue, &Op);
		} break;

		case AssetType_Font:
		{
			render_memory_op Op = {};
			Op.Type = RenderOp_LoadTexture;
			load_texture_op *LoadTexture = &Op.LoadTexture;
			LoadTexture->Texture = Asset->Font;
			LoadTexture->Pixels = WorkData->Dest;
			AddRenderOp(WorkData->RenderOpQueue, &Op);
		} break;

		case AssetType_Mesh:
		{
			render_memory_op Op = {};
			Op.Type = RenderOp_LoadMesh;
			load_mesh_op *LoadMesh = &Op.LoadMesh;
			LoadMesh->Mesh = Asset->Mesh;
			LoadMesh->VertexMemory = WorkData->Dest;
			LoadMesh->FaceMemory = WorkData->Dest + WorkData->VerticesSize;
			AddRenderOp(WorkData->RenderOpQueue, &Op);
		} break;

		InvalidDefaultCase;
	}

	Asset->State = Asset_Loaded;
}

WORK_QUEUE_CALLBACK(LoadAssetWork)
{
	load_asset_data *WorkData = (load_asset_data *) Data;

	LoadAssetImmediate(WorkData);

	EndTaskWithMemory(WorkData->TaskMemory);
}

internal void
LoadBitmap(assets *Assets, bitmap_id ID)
{
	if(ID.Value)
	{
		asset *Asset = Assets->Assets + ID.Value;
		Assert(Asset->Type == AssetType_Bitmap);

		if(AtomicCompareExchangeU32((u32 *)&Asset->State, Asset_Queued, Asset_Unloaded) == Asset_Unloaded)
		{
			task_memory *TaskMemory = BeginTaskWithMemory(Assets->GameState);

			if(TaskMemory)
			{
				bitmap_sla_v3 *BitmapSLA = &Asset->BitmapSLA;

				Asset->MemoryHeader = GetAssetMemoryBlock(Assets, SafeTruncateU64ToU32(Asset->DataSize));
				u8 *PixelDest = (u8 *) (Asset->MemoryHeader + 1);

				load_asset_data *Data = PushStruct(&TaskMemory->Region, load_asset_data);
				Data->TaskMemory = TaskMemory;
				Data->Asset = Asset;
				Data->AssetFile = &Asset->AssetFile->FileHandle;
				Data->Offset = Asset->DataOffset;
				Data->Size = Asset->DataSize;
				Data->Dest = PixelDest;
				Data->RenderOpQueue = Assets->RenderOpQueue;
				Asset->Bitmap = RendererTexture(Assets->NextFreeTextureHandle++, BitmapSLA->Width, BitmapSLA->Height);

				CompletePreviousWritesBeforeFutureWrites;
				Platform->AddWork(Assets->GameState->LowPriorityQueue, LoadAssetWork, Data);

				DLIST_INSERT(&Assets->LoadedAssetsSentinel, Asset);

#if GAME_DEBUG
				Outf(Assets->Info, "Asset loaded (%s)", WrapZ(Names_asset_name[Asset->Name]));
#endif
			}
			else
			{
				Asset->State = Asset_Unloaded;
			}
		}
	}
}

internal void
LoadMesh(assets *Assets, mesh_id ID)
{
	asset *Asset = Assets->Assets + ID.Value;
	Assert(Asset->Type == AssetType_Mesh);

	if(AtomicCompareExchangeU32((u32 *)&Asset->State, Asset_Queued, Asset_Unloaded) == Asset_Unloaded)
	{
		task_memory *TaskMemory = BeginTaskWithMemory(Assets->GameState);

		if(TaskMemory)
		{
			mesh_sla_v3 *MeshSLA = &Asset->MeshSLA;

			Asset->MemoryHeader = GetAssetMemoryBlock(Assets, Asset->DataSize);
			u8 *DataBuffer = (u8 *) (Asset->MemoryHeader + 1);

			load_asset_data *Data = PushStruct(&TaskMemory->Region, load_asset_data);
			Data->TaskMemory = TaskMemory;
			Data->Asset = Asset;
			Data->AssetFile = &Asset->AssetFile->FileHandle;
			Data->Offset = Asset->DataOffset;
			Data->Size = Asset->DataSize;
			Data->Dest = DataBuffer;
			Data->VerticesSize = SafeTruncateU64ToU32(MeshSLA->VerticesSize);
			Data->RenderOpQueue = Assets->RenderOpQueue;
			Asset->Mesh = RendererMesh(Assets->NextFreeMeshHandle++, MeshSLA->VertexCount, MeshSLA->FaceCount);

			CompletePreviousWritesBeforeFutureWrites;
			Platform->AddWork(Assets->GameState->LowPriorityQueue, LoadAssetWork, Data);

			DLIST_INSERT(&Assets->LoadedAssetsSentinel, Asset);

#if GAME_DEBUG
			Outf(Assets->Info, "Asset loaded (%s)", WrapZ(Names_asset_name[Asset->Name]));
#endif
		}
		else
		{
			Asset->State = Asset_Unloaded;
		}
	}
}

internal void
LoadFont(assets *Assets, font_id ID)
{
	asset *Asset = Assets->Assets + ID.Value;
	Assert(Asset->Type == AssetType_Font);

	if(AtomicCompareExchangeU32((u32 *)&Asset->State, Asset_Queued, Asset_Unloaded) == Asset_Unloaded)
	{
		task_memory *TaskMemory = BeginTaskWithMemory(Assets->GameState);

		if(TaskMemory)
		{
			font_sla_v3 *FontSLA = GetFontInfo(Assets, ID);
			bitmap_sla_v3 *BitmapSLA = &FontSLA->BitmapSLA;

			Asset->MemoryHeader = GetAssetMemoryBlock(Assets, Asset->DataSize);
			u8 *PixelDest = (u8 *) (Asset->MemoryHeader + 1);

			load_asset_data *Data = PushStruct(&TaskMemory->Region, load_asset_data);
			Data->TaskMemory = TaskMemory;
			Data->Asset = Asset;
			Data->AssetFile = &Asset->AssetFile->FileHandle;
			Data->Offset = Asset->DataOffset;
			Data->Size = Asset->DataSize;
			Data->Dest = PixelDest;
			Data->RenderOpQueue = Assets->RenderOpQueue;
			Asset->Bitmap = RendererTexture(Assets->NextFreeTextureHandle++, BitmapSLA->Width, BitmapSLA->Height);

			CompletePreviousWritesBeforeFutureWrites;
			Platform->AddWork(Assets->GameState->LowPriorityQueue, LoadAssetWork, Data);

			DLIST_INSERT(&Assets->LoadedAssetsSentinel, Asset);

#if GAME_DEBUG
			Outf(Assets->Info, "Asset loaded (%s)", WrapZ(Names_asset_name[Asset->Name]));
#endif
		}
		else
		{
			Asset->State = Asset_Unloaded;
		}
	}
}

internal void
LoadAssetMetadata(game_state *GameState, assets *Assets)
{
	memory_region *FrameRegion = &GameState->FrameRegion;

	// NOTE: Reserve space for a null asset.
	Assets->AssetCount = 1;
	Assets->NextAssetID = 1;

	platform_file_list AssetFiles = Platform->OpenAllFiles(ConstZ(".sla"));
	Assets->AssetFileCount = AssetFiles.Count;
	Assets->AssetFiles = PushArray(&Assets->Region, Assets->AssetFileCount, asset_file);

	for(u32 FileIndex = 0;
		FileIndex < Assets->AssetFileCount;
		++FileIndex)
	{
		asset_file *AssetFile = Assets->AssetFiles + FileIndex;
		AssetFile->FileHandle = AssetFiles.Files[FileIndex];
		AssetFile->Errors = CreateStream(&Assets->Region, 0);

		header_sla BaseHeader = {};
		Platform->ReadFromFile((u8 *)&BaseHeader, &AssetFile->FileHandle, 0, sizeof(header_sla));

		if(BaseHeader.MagicCode == MAGIC_CODE)
		{
			if(BaseHeader.Version == 3)
			{
				header_sla_v3 *Header = &AssetFile->Header;

				Platform->ReadFromFile((u8 *)Header, &AssetFile->FileHandle, 0, sizeof(header_sla_v3));
				AssetFile->TotalAssetCount = Header->TextureCount + Header->FontCount +
					Header->MeshCount + Header->EntityInfoCount;
				u32 AssetArraySize = (AssetFile->TotalAssetCount*sizeof(asset_header_sla_v3));
				if(Header->AssetHeadersSize == AssetArraySize)
				{
					AssetFile->IsValid = true;
					Assets->AssetCount += AssetFile->TotalAssetCount;
				}
				else
				{
					Outf(&AssetFile->Errors, "Asset header array is the wrong size! (Should be %d, but was %d)",
						AssetArraySize, Header->AssetHeadersSize);
					InvalidCodePath;
				}

				if(StringCompare(AssetFile->FileHandle.Name, "local.sla"))
				{
					AssetFile->CanBeWritten = true;
					Assets->LocalAssetFile = AssetFile;
				}
			}
			else
			{
				Outf(&AssetFile->Errors, "Version mismatch!");
			}
		}
		else
		{
			Outf(&AssetFile->Errors, "File corrupted!");
		}
	}

	Assets->Assets = PushArray(&Assets->Region, Assets->AssetCount, asset);

	for(u32 FileIndex = 0;
		FileIndex < Assets->AssetFileCount;
		++FileIndex)
	{
		asset_file *AssetFile = Assets->AssetFiles + FileIndex;
		if(AssetFile->IsValid)
		{
			header_sla_v3 *Header = &AssetFile->Header;

			u8 *MetaData = PushSize(FrameRegion, Header->MetaDataSize);
			Platform->ReadFromFile(MetaData, &AssetFile->FileHandle,
				SafeTruncateU64ToU32(Header->MetaDataOffset),
				SafeTruncateU64ToU32(Header->MetaDataSize));

			asset_header_sla_v3 *AssetHeaders = PushArray(FrameRegion, AssetFile->TotalAssetCount, asset_header_sla_v3);
			Platform->ReadFromFile((u8 *)AssetHeaders, &AssetFile->FileHandle,
				SafeTruncateU64ToU32(Header->AssetHeadersOffset),
				SafeTruncateU64ToU32(Header->AssetHeadersSize));

			for(u32 AssetIndex = 0;
				AssetIndex < AssetFile->TotalAssetCount;
				++AssetIndex)
			{
				asset_header_sla_v3 *AssetHeader = AssetHeaders + AssetIndex;
				u32 ID = Assets->NextAssetID++;
				asset *Asset = Assets->Assets + ID;
				Asset->ID = ID;
				Asset->State = Asset_Unloaded;

				Asset->Name = AssetHeader->Name;
				Asset->Type = AssetHeader->Type;
				Asset->DataOffset = SafeTruncateU64ToU32(Header->DataOffset + AssetHeader->DataOffset);
				Asset->DataSize = SafeTruncateU64ToU32(AssetHeader->DataSize);
				Asset->MetaDataOffset = SafeTruncateU64ToU32(Header->MetaDataOffset + AssetHeader->MetaDataOffset);
				Asset->MetaDataSize = SafeTruncateU64ToU32(AssetHeader->MetaDataSize);
				Asset->AssetFile = AssetFile;
				switch(AssetHeader->Type)
				{
					case AssetType_Bitmap:
					{
						bitmap_sla_v3 *Bitmap = (bitmap_sla_v3 *)(MetaData + AssetHeader->MetaDataOffset);
						if(AssetHeader->MetaDataSize != sizeof(bitmap_sla_v3))
						{
							Outf(&AssetFile->Errors, "Wrong metadata size! (Should be %d, but was %d)",
								sizeof(bitmap_sla_v3), AssetHeader->MetaDataSize);
						}

						Asset->BitmapSLA = *Bitmap;
						u32 DataSize = Bitmap->Width*Bitmap->Height*sizeof(u32);
						if(DataSize != AssetHeader->DataSize)
						{
							Outf(&AssetFile->Errors, "Wrong data size! (Should be %d, but was %d)",
								DataSize, AssetHeader->DataSize);
							InvalidCodePath;
						}
					} break;

					case AssetType_Font:
					{
						font_sla_v3 *Font = (font_sla_v3 *)(MetaData + AssetHeader->MetaDataOffset);
						if(AssetHeader->MetaDataSize != sizeof(font_sla_v3))
						{
							Outf(&AssetFile->Errors, "Wrong metadata size! (Should be %d, but was %d)",
								sizeof(font_sla_v3), AssetHeader->MetaDataSize);
						}

						Assert(Assets->FontInfoCount < ArrayCount(Assets->FontInfos));
						Asset->FontIndex = Assets->FontInfoCount++;
						font_sla_v3 *FontInfoSaved = Assets->FontInfos + Asset->FontIndex;
						*FontInfoSaved = *Font;

						asset_header_sla_v3 *BitmapHeader = &Font->BitmapHeader;
						bitmap_sla_v3 *BitmapData = &Font->BitmapSLA;
						if(BitmapHeader->MetaDataSize != sizeof(bitmap_sla_v3))
						{
							Outf(&AssetFile->Errors, "Wrong metadata size! (Should be %d, but was %d)",
								sizeof(bitmap_sla_v3), BitmapHeader->MetaDataSize);
						}

						Asset->DataOffset = SafeTruncateU64ToU32(Header->DataOffset + BitmapHeader->DataOffset);
						Asset->DataSize = SafeTruncateU64ToU32(BitmapHeader->DataSize);

						u32 DataSize = BitmapData->Width*BitmapData->Height*sizeof(u32);
						if(DataSize != BitmapHeader->DataSize)
						{
							Outf(&AssetFile->Errors, "Wrong data size! (Should be %d, but was %d)",
								DataSize, BitmapHeader->DataSize);
						}
					} break;

					case AssetType_Mesh:
					{
						mesh_sla_v3 *Mesh = (mesh_sla_v3 *)(MetaData + AssetHeader->MetaDataOffset);
						if(AssetHeader->MetaDataSize != sizeof(mesh_sla_v3))
						{
							Outf(&AssetFile->Errors, "Wrong metadata size! (Should be %d, but was %d)",
								sizeof(mesh_sla_v3), AssetHeader->MetaDataSize);
						}
						Asset->MeshSLA = *Mesh;

						u32 VerticesSize = Mesh->VertexCount*sizeof(vertex_format);
						u32 FacesSize = Mesh->FaceCount*sizeof(v3u);
						u32 DataSize = VerticesSize + FacesSize;
						Assert(VerticesSize == Mesh->VerticesSize);
						Assert(FacesSize == Mesh->FacesSize);
						if(AssetHeader->DataSize != DataSize)
						{
							Outf(&AssetFile->Errors, "Wrong data size! (Should be %d, but was %d)",
								DataSize, AssetHeader->DataSize);
							InvalidCodePath;
						}
					} break;

					case AssetType_EntityInfo:
					{
						entity_info *Info = (entity_info *)(MetaData + AssetHeader->MetaDataOffset);
						if(AssetHeader->MetaDataSize != sizeof(entity_info))
						{
							Outf(&AssetFile->Errors, "Wrong metadata size! (Should be %d, but was %d)",
								sizeof(entity_info), AssetHeader->MetaDataSize);
						}

						Asset->EntityInfo = Assets->EntityInfos + Info->Type;
						*Asset->EntityInfo = *Info;
						Asset->EntityInfo->DisplayName.Text = Asset->EntityInfo->Buffer;
					} break;

					InvalidDefaultCase;
				}
			}
		}
	}
}

internal void
WriteDirtyAssets(assets *Assets, memory_region *FrameRegion)
{
	for(u32 AssetIndex = 0;
		AssetIndex < Assets->AssetCount;
		++AssetIndex)
	{
		asset *Asset = Assets->Assets + AssetIndex;
		asset_file *File = Asset->AssetFile;

		if(Asset->IsDirty &&
		   File->IsValid &&
		   File->CanBeWritten)
		{
			switch(Asset->Type)
			{
				case AssetType_EntityInfo:
				{
					Platform->WriteIntoFile(Asset->EntityInfo, &File->FileHandle,
						Asset->MetaDataOffset,
						Asset->MetaDataSize);
				} break;

				InvalidDefaultCase;
			}

			Asset->IsDirty = false;
		}
	}
}

internal void
WriteEntityInfo(assets *Assets, memory_region *FrameRegion)
{
	asset_file *AssetFile = Assets->LocalAssetFile;
	if(AssetFile &&
	   AssetFile->IsValid &&
	   AssetFile->CanBeWritten)
	{
		header_sla_v3 *Header = &AssetFile->Header;
		asset_header_sla_v3 *OldAssetHeaders = PushArray(FrameRegion, AssetFile->TotalAssetCount, asset_header_sla_v3);
		Platform->ReadFromFile(OldAssetHeaders, &AssetFile->FileHandle,
			SafeTruncateU64ToU32(Header->AssetHeadersOffset),
			SafeTruncateU64ToU32(Header->AssetHeadersSize));

		u32 NewAssetHeadersSize = Entity_Count*sizeof(asset_header_sla_v3);
		asset_header_sla_v3 *NewAssetHeaders = PushArray(FrameRegion, Entity_Count, asset_header_sla_v3);

		for(u32 InfoIndex = 0;
			InfoIndex < Entity_Count;
			++InfoIndex)
		{
			asset_header_sla_v3 *AssetHeader = NewAssetHeaders + InfoIndex;
			AssetHeader->Type = AssetType_EntityInfo;
			AssetHeader->Name = Asset_EntityInfo;
			AssetHeader->MetaDataOffset = Header->MetaDataSize;
			AssetHeader->MetaDataSize = sizeof(entity_info);

			entity_info *Info = Assets->EntityInfos + InfoIndex;
			Platform->WriteIntoFile(Info, &AssetFile->FileHandle,
				SafeTruncateU64ToU32(Header->MetaDataOffset + Header->MetaDataSize),
				SafeTruncateU64ToU32(AssetHeader->MetaDataSize));

			++Header->EntityInfoCount;
			Header->MetaDataSize += AssetHeader->MetaDataSize;
		}

		Header->AssetHeadersOffset = Header->MetaDataOffset + Header->MetaDataSize;
		Platform->WriteIntoFile(OldAssetHeaders, &AssetFile->FileHandle,
			SafeTruncateU64ToU32(Header->AssetHeadersOffset),
			SafeTruncateU64ToU32(Header->AssetHeadersSize));

		Platform->WriteIntoFile(NewAssetHeaders, &AssetFile->FileHandle,
			SafeTruncateU64ToU32(Header->AssetHeadersOffset + Header->AssetHeadersSize),
			NewAssetHeadersSize);
		Header->AssetHeadersSize += NewAssetHeadersSize;

		Platform->WriteIntoFile(Header, &AssetFile->FileHandle,
			0, sizeof(header_sla_v3));
	}
}

internal asset *
GetEntityInfoAsset(assets *Assets, entity_info *Info)
{
	asset *Result = 0;

	for(u32 Index = 0;
		Index < Assets->AssetCount;
		++Index)
	{
		asset *TestAsset = Assets->Assets + Index;
		if(TestAsset->EntityInfo == Info)
		{
			Result = TestAsset;
			break;
		}
	}

	return Result;
}

internal entity_info *
GetEntityInfo(assets *Assets, entity_type EntityType)
{
	entity_info *Result = Assets->EntityInfos + EntityType;
	return Result;
}

internal void
AddPiece(entity_info *Entity, v3 Dim, quaternion Rotation, v3 Offset, entity_visible_piece_type Type,
	bitmap_id BitmapID, mesh_id MeshID, surface_material Material)
{
	Assert(Entity->PieceCount < ArrayCount(Entity->Pieces));
	entity_visible_piece *Piece = Entity->Pieces + Entity->PieceCount++;
	Piece->Dim = Dim;
	Piece->Rotation = Rotation;
	Piece->Offset = Offset;
	Piece->Type = Type;
	Piece->BitmapID = BitmapID;
	Piece->MeshID = MeshID;
	Piece->Material = Material;
}

internal void
AddPieceBitmap(entity_info *EntityInfo, v2 Scale, quaternion Rotation, v3 Offset,
	bitmap_id BitmapID, surface_material Material)
{
	AddPiece(EntityInfo, V3(Scale.x, Scale.y, 0), Rotation, Offset, PieceType_Bitmap, BitmapID, {0}, Material);
}

internal void
AddPieceBitmapUpright(entity_info *EntityInfo, v2 Scale, v3 Offset,
	bitmap_id BitmapID, surface_material Material)
{
	// TODO: Add rotation!
	AddPiece(EntityInfo, V3(Scale.x, Scale.y, 0), Q(1,0,0,0), Offset, PieceType_Bitmap_Upright, BitmapID, {0}, Material);
}

internal void
AddPieceCube(entity_info *EntityInfo, v3 Dim, quaternion Rotation, v3 Offset,
	bitmap_id BitmapID, surface_material Material)
{
	AddPiece(EntityInfo, Dim, Rotation, Offset, PieceType_Cube, BitmapID, {0}, Material);
}

internal void
AddPieceMesh(entity_info *EntityInfo, v3 Scale, quaternion Rotation, v3 Offset,
	bitmap_id BitmapID, mesh_id MeshID, surface_material Material)
{
	AddPiece(EntityInfo, Scale, Rotation, Offset, PieceType_Mesh, BitmapID, MeshID, Material);
}

internal void
AddPieceEntityInfo(entity_info *EntityInfo, v2 Scale, quaternion Rotation, v3 Offset, surface_material Material)
{
	AddPiece(EntityInfo, V3(Scale.x, Scale.y, 0), Rotation, Offset, PieceType_EntityInfo, {0}, {0}, Material);
}

internal void
SetEntityInfoName(entity_info *Info, char *Name)
{
	Info->DisplayName = FormatString(Info->Buffer, ArrayCount(Info->Buffer), Name);
}

internal void
InitBasicEntityInfos(assets *Assets, v3 GroundDim, v3 CardDim)
{
	v3 Dim = GroundDim;
	v3 LocalGroundP = V3(0, 0.5f*Dim.y, 0);

	entity_info DefaultBuilding = {};
	DefaultBuilding.Color = V4(1,1,1,1);
	surface_material BuildingMaterial = {};
	BuildingMaterial.Roughness = 0.95f;
	BuildingMaterial.Metalness = 0.0f;
	BuildingMaterial.Color = V4(1,1,1,1);
	f32 BuildingHeight = 0.8f;
	AddPieceCube(&DefaultBuilding, V3(0.65f*Dim.x, BuildingHeight, 0.65f*Dim.x), NoRotation(),
		LocalGroundP + 0.5f*V3(0, BuildingHeight, 0), {0}, BuildingMaterial);
	DefaultBuilding.MaxDefense = 1;
	DefaultBuilding.Flags |= EntityFlag_IsBuilding|EntityFlag_OnBoard;

	entity_info DefaultUnit = {};
	DefaultUnit.Color = V4(1,1,1,1);
	surface_material UnitMaterial = {};
	UnitMaterial.Roughness = 0.5f;
	UnitMaterial.Metalness = 0.7f;
	UnitMaterial.Color = V4(1,1,1,1);
	f32 Height = 0.6f*Dim.x;
	AddPieceBitmapUpright(&DefaultUnit, V2(Height, Height), LocalGroundP + 0.75f*V3(0, Height, 0),
		GetBitmapID(Assets, Asset_SwordIcon), UnitMaterial);
	AddPieceCube(&DefaultUnit, 0.25f*V3(Dim.x, Dim.x, Dim.x), NoRotation(),
		LocalGroundP + 0.25f*V3(0, 0.5f*Dim.x, 0), {0}, UnitMaterial);
	DefaultUnit.MaxDefense = 1;
	DefaultUnit.Flags |= EntityFlag_IsUnit|EntityFlag_OnBoard;

	{
		entity_info *Info = Assets->EntityInfos + Entity_Ground;
		ZeroStruct(*Info);
		Info->Type = Entity_Ground;
		SetEntityInfoName(Info, "Ground");
		surface_material Material = {};
		Material.Roughness = 1.0f;
		Material.Metalness = 0.0f;
		Material.Color = V4(0.35f, 0.35f, 0.25f, 1.0f);
		AddPieceCube(Info, Dim, NoRotation(), {}, {0}, Material);
		Info->Flags |= EntityFlag_Traversable|EntityFlag_OnBoard;
	}

	{
		entity_info *Info = Assets->EntityInfos + Entity_Water;
		ZeroStruct(*Info);
		Info->Type = Entity_Water;
		SetEntityInfoName(Info, "Water");
		surface_material Material = {};
		Material.Roughness = 0.1f;
		Material.Metalness = 0.4f;
		Material.Color = V4(0.15f, 0.35f, 0.65f, 1.0f);
		AddPieceCube(Info, Dim, NoRotation(), {}, {0}, Material);
		Info->Flags |= EntityFlag_OnBoard;
	}

	{
		entity_info *Info = Assets->EntityInfos + Entity_Forest;
		ZeroStruct(*Info);
		Info->Type = Entity_Forest;
		SetEntityInfoName(Info, "Forest");
		Info->Flags |= EntityFlag_Traversable|EntityFlag_OnBoard;

		surface_material LeafMaterial = {};
		LeafMaterial.Roughness = 0.75f;
		LeafMaterial.Metalness = 0.0f;
		LeafMaterial.Color = V4(0.2f, 0.6f, 0.3f, 1.0f);
		f32 LeafHeight = 0.4f;
		AddPieceCube(Info, 0.3f*V3(Dim.x, Dim.x, Dim.x), NoRotation(),
		LocalGroundP + V3(0, LeafHeight, 0), {0}, LeafMaterial);

		surface_material WoodMaterial = {};
		WoodMaterial.Roughness = 0.95f;
		WoodMaterial.Metalness = 0.0f;
		WoodMaterial.Color = V4(0.4f, 0.3f, 0.2f, 1.0f);
		AddPieceCube(Info, V3(0.1f*Dim.x, LeafHeight, 0.1f*Dim.z), NoRotation(),
		LocalGroundP + 0.5f*V3(0, LeafHeight, 0), {0}, WoodMaterial);
	}

	{
		entity_info *Info = Assets->EntityInfos + Entity_Mountain;
		ZeroStruct(*Info);
		Info->Type = Entity_Mountain;
		SetEntityInfoName(Info, "Mountain");
		Info->Flags |= EntityFlag_OnBoard;
		surface_material StoneMaterial = {};
		StoneMaterial.Roughness = 0.95f;
		StoneMaterial.Metalness = 0.0f;
		StoneMaterial.Color = V4(0.6f, 0.6f, 0.6f, 1.0f);
		f32 CliffHeight = 0.2f;
		AddPieceCube(Info, V3(0.75f*Dim.x, CliffHeight, 0.75f*Dim.x), NoRotation(),
			LocalGroundP + 0.5f*V3(0, CliffHeight, 0), {0}, StoneMaterial);
		AddPieceCube(Info, V3(0.5f*Dim.x, CliffHeight, 0.5f*Dim.x), NoRotation(),
			LocalGroundP + 1.5f*V3(0, CliffHeight, 0), {0}, StoneMaterial);
		AddPieceCube(Info, V3(0.25f*Dim.x, CliffHeight, 0.25f*Dim.x), NoRotation(),
			LocalGroundP + 2.5f*V3(0, CliffHeight, 0), {0}, StoneMaterial);
	}

	{
		entity_info *Info = Assets->EntityInfos + Entity_Headquarters;
		ZeroStruct(*Info);
		*Info = DefaultBuilding;
		Info->Type = Entity_Headquarters;
		SetEntityInfoName(Info, "Headquarters");
		Info->MaxDefense = 5;
		Info->Influence = 2;
		Info->Abilities[Info->AbilityCount++] = Ability_GainGold;
		Info->Abilities[Info->AbilityCount++] = Ability_DrawCard;
		Info->Abilities[Info->AbilityCount++] = Ability_GatherResources;
		Info->Abilities[Info->AbilityCount++] = Ability_HQPillaged;
	}

	{
		entity_info *Info = Assets->EntityInfos + Entity_Warrior;
		ZeroStruct(*Info);
		*Info = DefaultUnit;
		Info->Type = Entity_Warrior;
		SetEntityInfoName(Info, "Warrior");
		Info->Attack = 2;
		Info->MaxDefense = 3;
		Info->Abilities[Info->AbilityCount++] = Ability_Move;
		Info->Abilities[Info->AbilityCount++] = Ability_Fireball;
	}

	{
		entity_info *Info = Assets->EntityInfos + Entity_Mercenaries;
		ZeroStruct(*Info);
		*Info = DefaultUnit;
		Info->Type = Entity_Mercenaries;
		SetEntityInfoName(Info, "Mercenaries");
		Info->Attack = 1;
		Info->MaxDefense = 4;
		Info->Abilities[Info->AbilityCount++] = Ability_Move;
		Info->Abilities[Info->AbilityCount++] = Ability_Attack;
		Info->Abilities[Info->AbilityCount++] = Ability_HireMercenaries;
	}

	{
		entity_info *Info = Assets->EntityInfos + Entity_Card;
		ZeroStruct(*Info);
		Info->Type = Entity_Card;
		SetEntityInfoName(Info, "Card");
		Info->Flags |= EntityFlag_InHand;

		surface_material FrontMaterial = {};
		FrontMaterial.Roughness = 1.0f;
		FrontMaterial.Metalness = 0.0f;
		FrontMaterial.Color = V4(1,1,1,1);

		surface_material BackMaterial = {};
		BackMaterial.Roughness = 1.0f;
		BackMaterial.Metalness = 0.0f;
		BackMaterial.Color = V4(0.2f,0.2f,0.2f,1);

		AddPieceEntityInfo(Info, CardDim.xy, NoRotation(), {}, FrontMaterial);
		AddPieceBitmap(Info, CardDim.xy, RotationQuaternion(V3(0,1,0), Pi32), {},
			GetBitmapID(Assets, Asset_HeartIcon), BackMaterial);
	}

	{
		entity_info *Info = Assets->EntityInfos + Entity_Bank;
		ZeroStruct(*Info);
		*Info = DefaultBuilding;
		Info->Type = Entity_Bank;
		SetEntityInfoName(Info, "Bank");
		Info->MaxDefense = 4;
		Info->Influence = 1;
		Info->Abilities[Info->AbilityCount++] = Ability_BankActive;
		Info->Abilities[Info->AbilityCount++] = Ability_BankPillaged;
	}

	{
		entity_info *Info = Assets->EntityInfos + Entity_RiskInvestment;
		ZeroStruct(*Info);
		*Info = DefaultBuilding;
		Info->Type = Entity_RiskInvestment;
		SetEntityInfoName(Info, "Risk Investment");
		Info->MaxDefense = 1;
		Info->Abilities[Info->AbilityCount++] = Ability_RiskInvestmentReturn;
		Info->Abilities[Info->AbilityCount++] = Ability_GenerateCharge;
		Info->Abilities[Info->AbilityCount++] = Ability_RiskInvestmentPillaged;
	}

	{
		entity_info *Info = Assets->EntityInfos + Entity_GoldMine;
		ZeroStruct(*Info);
		*Info = DefaultBuilding;
		Info->Type = Entity_GoldMine;
		SetEntityInfoName(Info, "Gold Mine");
		Info->MaxDefense = 2;
		Info->Abilities[Info->AbilityCount++] = Ability_GoldMine;
		Info->Abilities[Info->AbilityCount++] = Ability_GoldMinePillaged;
	}

	{
		entity_info *Info = Assets->EntityInfos + Entity_PowerPlant;
		ZeroStruct(*Info);
		*Info = DefaultBuilding;
		Info->Type = Entity_PowerPlant;
		SetEntityInfoName(Info, "Power Plant");
		Info->MaxDefense = 4;
		Info->Influence = 1;
		Info->Abilities[Info->AbilityCount++] = Ability_GeneratePower;
		Info->Abilities[Info->AbilityCount++] = Ability_PowerPlantPillaged;
	}

}

internal assets *
InitializeAssets(game_state *GameState, u32 TotalAssetSpace,
	render_memory_op_queue *RenderOpQueue, stream *Info)
{
	assets *Assets = BootstrapPushStruct(assets, Region, NonRestoredParams());
	Assets->GameState = GameState;
	Assets->RenderOpQueue = RenderOpQueue;
	Assets->Info = Info;

	Assets->NextFreeTextureHandle = 1;
	Assets->NextFreeMeshHandle = 1;

	DLIST_INIT(&Assets->MemorySentinel);
	Assets->MemorySentinel.InUse = true;

	entity_info DefaultInfo_ = {};
	entity_info *DefaultInfo = &DefaultInfo_;
	DefaultInfo->Type = Entity_None;
	SetEntityInfoName(DefaultInfo, "Default");
	DefaultInfo->Color = V4(1,1,1,1);
	DefaultInfo->PieceCount = 1;
	DefaultInfo->Pieces[0].Dim = V3(1,1,1);
	DefaultInfo->Pieces[0].Rotation = NoRotation();
	DefaultInfo->Pieces[0].Offset = V3(0,0,0);
	DefaultInfo->Pieces[0].Type = PieceType_Cube;
	DefaultInfo->Pieces[0].Material.Roughness = 1.0f;
	DefaultInfo->Pieces[0].Material.Metalness = 0.0f;
	DefaultInfo->Pieces[0].Material.Color = V4(1, 0, 1, 1);

	for(u32 InfoIndex = 0;
		InfoIndex < ArrayCount(Assets->EntityInfos);
		++InfoIndex)
	{
		entity_info *EntityInfo = Assets->EntityInfos + InfoIndex;
		*EntityInfo = *DefaultInfo;
	}

	LoadAssetMetadata(GameState, Assets);

	asset_memory_header *FirstBlock = (asset_memory_header *) PushSize(&Assets->Region, TotalAssetSpace);
	FirstBlock->InUse = false;
	FirstBlock->BlockSize = TotalAssetSpace - sizeof(asset_memory_header);
	DLIST_INSERT(&Assets->MemorySentinel, FirstBlock);

	DLIST_INIT(&Assets->LoadedAssetsSentinel);

#if 0
	v3 CardDim = V3(1.0f, 1.3f, 0.01f);
	v3 GroundDim = V3(1.0, 0.1f, 1.0f);
	InitBasicEntityInfos(Assets, GroundDim, CardDim);
	WriteEntityInfo(Assets, &GameState->FrameRegion);
#endif

	return Assets;
}
