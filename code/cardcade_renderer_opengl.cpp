/*@H
* File: cardcade_renderer_opengl.cpp
* Author: Jesse Calvert
* Created: November 27, 2017, 15:38
* Last modified: November 6, 2018, 13:17
*/

OPENGL_DEBUG_CALLBACK(OpenGLDebugCallback)
{
	stream *Info = (stream *)userParam;
	char *ErrorMessage = (char *)message;
	u32 Length = (u32)length;

	if(severity == GL_DEBUG_SEVERITY_HIGH)
	{
		Assert(!"OpenGL error.");
	}
	else if(severity == GL_DEBUG_SEVERITY_MEDIUM)
	{
		string OpenGLMessage = {Length, ErrorMessage};
		Outf(Info, "Severity Medium : %s", OpenGLMessage);
	}
	else if(severity == GL_DEBUG_SEVERITY_LOW)
	{
		string OpenGLMessage = {Length, ErrorMessage};
		Outf(Info, "Severity Low : %s", OpenGLMessage);
	}
}

internal opengl_info
OpenGLGetInfo()
{
	opengl_info Result = {};

	Result.Vendor = (char *)glGetString(GL_VENDOR);
	Result.Renderer = (char *)glGetString(GL_RENDERER);
	Result.Version = (char *)glGetString(GL_VERSION);
	Result.ShadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	Result.Extensions = (char *)glGetString(GL_EXTENSIONS);

	char *At = Result.Extensions;
	while(*At)
	{
		while(IsWhitespace(*At)) {++At;}
		char *End = At;
		while(*End && !IsWhitespace(*End)) {++End;}

		umm Count = End - At;

		if(0) {}
        else if(StringsAreEqual(Count, At, "GL_EXT_texture_sRGB")) {Result.GL_EXT_texture_sRGB=true;}
        else if(StringsAreEqual(Count, At, "GL_EXT_framebuffer_sRGB")) {Result.GL_EXT_framebuffer_sRGB=true;}
        else if(StringsAreEqual(Count, At, "GL_ARB_framebuffer_sRGB")) {Result.GL_EXT_framebuffer_sRGB=true;}

		At = End;
	}

	return Result;
}

internal void
OpenGLSetUpVertexAttribs()
{
	// NOTE: A vertex array should be bound first!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_format), (GLvoid *)OffsetOf(vertex_format, Position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_format), (GLvoid *)OffsetOf(vertex_format, Normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_format), (GLvoid *)OffsetOf(vertex_format, TexCoords));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex_format), (GLvoid *)OffsetOf(vertex_format, Color));
	glEnableVertexAttribArray(3);

	glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(vertex_format), (GLvoid *)OffsetOf(vertex_format, TextureIndex));
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_format), (GLvoid *)OffsetOf(vertex_format, Roughness_Metalness));
	glEnableVertexAttribArray(5);
}

internal void
OpenGLFreeMesh(opengl_state *State, opengl_mesh_info *MeshInfo)
{
	if(MeshInfo->Mesh.Index)
	{
		glDeleteVertexArrays(1, &MeshInfo->VAO);
		glDeleteBuffers(1, &MeshInfo->VBO);
		glDeleteBuffers(1, &MeshInfo->EBO);
		MeshInfo->Mesh = {};
	}
}

internal void
OpenGLLoadMesh(opengl_state *State, vertex_format *Vertices, v3u *Faces, renderer_mesh Mesh)
{
	opengl_mesh_info *MeshInfo = State->Meshes + Mesh.Index;
	OpenGLFreeMesh(State, MeshInfo);
	MeshInfo->Mesh = Mesh;
	MeshInfo->VertexCount = Mesh.VertexCount;
	MeshInfo->FaceCount = Mesh.FaceCount;

	glGenVertexArrays(1, &MeshInfo->VAO);
	glGenBuffers(1, &MeshInfo->VBO);
	glGenBuffers(1, &MeshInfo->EBO);

	glBindVertexArray(MeshInfo->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, MeshInfo->VBO);
	glBufferData(GL_ARRAY_BUFFER, Mesh.VertexCount*sizeof(vertex_format), Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MeshInfo->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Mesh.FaceCount*sizeof(v3u), Faces, GL_STATIC_DRAW);

	OpenGLSetUpVertexAttribs();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

internal texture_settings
TextureSettings(u32 Width, u32 Height, u32 Channels, u32 BytesPerChannel, flag32(texture_flags) Flags, u32 Samples = 0, v4 BorderColor = V4(1,1,1,1))
{
	texture_settings Settings = {};
	Settings.Width = Width;
	Settings.Height = Height;
	Settings.BorderColor = BorderColor;
	Settings.Channels = Channels;
	Settings.BytesPerChannel = BytesPerChannel;
	Settings.Samples = Samples;
	Settings.Flags = Flags;
	return Settings;
}

internal GLuint
OpenGLAllocateAndLoadTexture(texture_settings Settings, void *Pixels = 0)
{
	Assert(!((Settings.Flags & Texture_ClampToBorder) &&
			 (Settings.Flags & Texture_ClampToEdge)));
	Assert(!((Settings.Flags & Texture_FilterNearest) &&
			 (Settings.Flags & Texture_Mipmap)));
	if(!(Settings.Flags & Texture_Float))
	{
		Assert((Settings.Channels == 1) || (Settings.Channels == 4));
	}

	GLuint Result;
	GLenum TextureTarget = (Settings.Flags & Texture_Multisample) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

	glGenTextures(1, &Result);
	glBindTexture(TextureTarget, Result);

	GLenum WrapParam = GL_REPEAT;
	if(Settings.Flags & Texture_ClampToBorder)
	{
		WrapParam = GL_CLAMP_TO_BORDER;
		glTexParameterfv(TextureTarget, GL_TEXTURE_BORDER_COLOR, Settings.BorderColor.E);
	}
	else if(Settings.Flags & Texture_ClampToEdge)
	{
		WrapParam = GL_CLAMP_TO_EDGE;
	}

	GLenum MinFilter = GL_LINEAR;
	GLenum MagFilter = GL_LINEAR;
	if(Settings.Flags & Texture_FilterNearest)
	{
		MinFilter = GL_NEAREST;
		MagFilter = GL_NEAREST;
	}
	else if(Settings.Flags & Texture_Mipmap)
	{
		MinFilter = GL_LINEAR_MIPMAP_LINEAR;
	}

	if(!(Settings.Flags & Texture_Multisample))
	{
	    glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_S, WrapParam);
	    glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_T, WrapParam);
	    glTexParameteri(TextureTarget, GL_TEXTURE_MIN_FILTER, MinFilter);
	    glTexParameteri(TextureTarget, GL_TEXTURE_MAG_FILTER, MagFilter);
	}

	GLint InternalFormat = 0;
	GLenum PixelFormat = 0;
	GLenum PixelType = 0;

    if(Settings.Flags & Texture_DepthBuffer)
    {
    	Assert(Settings.Flags & Texture_Float);
    	Assert(Settings.Channels == 1);
    	Assert(Settings.BytesPerChannel == 4);
    	InternalFormat = GL_DEPTH_COMPONENT32F;
    	PixelFormat = GL_DEPTH_COMPONENT;
    	PixelType = GL_FLOAT;
    }
    else
    {
    	b32 Float = Settings.Flags & Texture_Float;

    	switch(Settings.Channels)
    	{
    		case 1:
    		{
    			if(Float)
    			{
    				PixelType = GL_FLOAT;
    				switch(Settings.BytesPerChannel)
    				{
    					case 2:
    					{
							InternalFormat = GL_R16F;
    					} break;

    					case 4:
    					{
							InternalFormat = GL_R32F;
    					} break;

    					InvalidDefaultCase;
    				}
    			}
    			else
    			{
					InternalFormat = GL_RED;
					switch(Settings.BytesPerChannel)
    				{
    					case 1:
    					{
    						PixelType = GL_UNSIGNED_BYTE;
    					} break;

    					case 4:
    					{
    						PixelType = GL_UNSIGNED_INT;
    					} break;

    					InvalidDefaultCase;
    				}
    			}

    			PixelFormat = GL_RED;
    		} break;

    		case 2:
    		{
    			if(Float)
    			{
    				PixelType = GL_FLOAT;
    				switch(Settings.BytesPerChannel)
    				{
    					case 2:
    					{
							InternalFormat = GL_RG16F;
    					} break;

    					case 4:
    					{
							InternalFormat = GL_RG32F;
    					} break;

    					InvalidDefaultCase;
    				}
    			}
    			else
    			{
					InternalFormat = GL_RG;
					switch(Settings.BytesPerChannel)
    				{
    					case 1:
    					{
    						PixelType = GL_UNSIGNED_SHORT;
    					} break;

    					InvalidDefaultCase;
    				}
    			}

    			PixelFormat = GL_RG;
    		} break;

    		case 3:
    		{
    			if(Float)
    			{
    				PixelType = GL_FLOAT;
    				switch(Settings.BytesPerChannel)
    				{
    					case 2:
    					{
							InternalFormat = GL_RGB16F;
    					} break;

    					case 4:
    					{
							InternalFormat = GL_RGB32F;
    					} break;

    					InvalidDefaultCase;
    				}
    			}
    			else
    			{
					InternalFormat = GL_RGB;
					switch(Settings.BytesPerChannel)
    				{
    					case 1:
    					{
    						PixelType = GL_UNSIGNED_INT;
    					} break;

    					InvalidDefaultCase;
    				}
    			}

    			PixelFormat = GL_RGB;
    		} break;

    		case 4:
    		{
				if(Float)
    			{
    				PixelType = GL_FLOAT;
    				switch(Settings.BytesPerChannel)
    				{
    					case 2:
    					{
							InternalFormat = GL_RGBA16F;
    					} break;

    					case 4:
    					{
							InternalFormat = GL_RGBA32F;
    					} break;

    					InvalidDefaultCase;
    				}
    			}
    			else
    			{
					InternalFormat = GL_RGBA;
					switch(Settings.BytesPerChannel)
    				{
    					case 1:
    					{
    						PixelType = GL_UNSIGNED_INT_8_8_8_8;
    					} break;

    					InvalidDefaultCase;
    				}
    			}

    			PixelFormat = GL_RGBA;
    		} break;

    		InvalidDefaultCase;
    	}
    }

    if(Settings.Flags & Texture_Multisample)
    {
		glTexImage2DMultisample(TextureTarget, Settings.Samples, InternalFormat, Settings.Width, Settings.Height, GL_TRUE);
    }
    else
    {
		glTexImage2D(TextureTarget, 0, InternalFormat, Settings.Width, Settings.Height, 0, PixelFormat, PixelType, Pixels);
    }

	if(Settings.Flags & Texture_Mipmap)
	{
		glGenerateMipmap(TextureTarget);
	}

	glBindTexture(TextureTarget, 0);

	return Result;
}

internal void
OpenGLLoadTexture(opengl_state *State, void *Pixels, renderer_texture Texture)
{
#if 0
	texture_settings Settings = TextureSettings(Width, Height, 4, 1, 0);

	u64 TextureHandle = OpenGLAllocateAndLoadTexture(Settings, Pixels);
	u32 TextureHandle32 = (u32) TextureHandle;
	Assert(TextureHandle32 == TextureHandle);

	renderer_texture Result = {};
	Result.Index = TextureHandle32;
	Result.Width = Width;
	Result.Height = Height;
	return Result;

#else
	Assert(Texture.Width <= MAX_TEXTURE_DIM);
	Assert(Texture.Height <= MAX_TEXTURE_DIM);
	Assert(Texture.Index < State->MaxTextureCount);

	glBindTexture(GL_TEXTURE_2D_ARRAY, State->TextureArray);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, Texture.Index,
		Texture.Width, Texture.Height, 1,
		GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, Pixels);

#endif
}

internal void
OpenGLSetShaderFilenames(opengl_state *State)
{
	State->ShaderFilenames[Shader_Header] = ConstZ("Shaders/cardcade_header.glsl");
	State->ShaderFilenames[Shader_Scene_Quads] = ConstZ("Shaders/cardcade_scene.glsl");
	State->ShaderFilenames[Shader_Scene_Quads_SDF] = ConstZ("Shaders/cardcade_scene.glsl");
	State->ShaderFilenames[Shader_Scene_Meshes] = ConstZ("Shaders/cardcade_scene.glsl");
	State->ShaderFilenames[Shader_ResolveDepthPeel] = ConstZ("Shaders/cardcade_resolve_depth_peel.glsl");
	State->ShaderFilenames[Shader_CompositeDepthPeel] = ConstZ("Shaders/cardcade_composite_depth_peel.glsl");
	State->ShaderFilenames[Shader_Overlay] = ConstZ("Shaders/cardcade_overlay.glsl");
	State->ShaderFilenames[Shader_Overlay_SDF] = ConstZ("Shaders/cardcade_overlay.glsl");
	State->ShaderFilenames[Shader_Overbright] = ConstZ("Shaders/cardcade_overbright.glsl");
	State->ShaderFilenames[Shader_Blur] = ConstZ("Shaders/cardcade_blur.glsl");
	State->ShaderFilenames[Shader_Finalize] = ConstZ("Shaders/cardcade_finalize.glsl");
	State->ShaderFilenames[Shader_DebugView] = ConstZ("Shaders/cardcade_debug_view.glsl");
}

internal opengl_state *
InitOpenGL(stream *Info)
{
	opengl_state *State = BootstrapPushStruct(opengl_state, Region, NonRestoredParams());

	State->OpenGLInfo = OpenGLGetInfo();
	State->Info = Info;
	OpenGLSetShaderFilenames(State);

#if GAME_DEBUG
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(OpenGLDebugCallback, Info);
#endif

	// TODO: Pass these in
	State->MaxTextureCount = 32;
	State->MaxMeshCount = 32;
	State->Meshes = PushArray(&State->Region, State->MaxMeshCount, opengl_mesh_info);

	glEnable(GL_MULTISAMPLE);

	glDepthRange(1.0f, 0.0f);
	glCullFace(GL_BACK);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &State->OpenGLInfo.MaxSamplersPerShader);

	glGenVertexArrays(1, &State->VertexArray);
	glBindVertexArray(State->VertexArray);

	glGenBuffers(1, &State->ArrayBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, State->ArrayBuffer);

	glGenBuffers(1, &State->IndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, State->IndexBuffer);

	OpenGLSetUpVertexAttribs();

	glBindVertexArray(0);

	glGenTextures(1, &State->TextureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, State->TextureArray);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, MAX_TEXTURE_DIM, MAX_TEXTURE_DIM, State->MaxTextureCount);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	vertex_format BasicQuadVertices[] =
	{
		{{0.0f, 0.0f, 0.0f}, {0,0,1}, {0.0f, 0.0f}, {0xFFFFFFFF}},
		{{0.0f, 1.0f, 0.0f}, {0,0,1}, {0.0f, 1.0f}, {0xFFFFFFFF}},
		{{1.0f, 1.0f, 0.0f}, {0,0,1}, {1.0f, 1.0f}, {0xFFFFFFFF}},
		{{1.0f, 0.0f, 0.0f}, {0,0,1}, {1.0f, 0.0f}, {0xFFFFFFFF}},
	};
	v3u BasicQuadFaces[] =
	{
		{0, 3, 2},
		{0, 2, 1},
	};

	// TODO: These are taking up indexes that the app could want to use!
	State->Quad = RendererMesh(25, ArrayCount(BasicQuadVertices), ArrayCount(BasicQuadFaces));
	OpenGLLoadMesh(State, BasicQuadVertices, BasicQuadFaces, State->Quad);

	vertex_format BasicCubeVertices[] =
	{
		{{-1.0f, -1.0f, -1.0f}, {}, {}, {0xFFFFFFFF}},
		{{ 1.0f, -1.0f, -1.0f}, {}, {}, {0xFFFFFFFF}},
		{{-1.0f,  1.0f, -1.0f}, {}, {}, {0xFFFFFFFF}},
		{{-1.0f, -1.0f,  1.0f}, {}, {}, {0xFFFFFFFF}},
		{{-1.0f,  1.0f,  1.0f}, {}, {}, {0xFFFFFFFF}},
		{{ 1.0f, -1.0f,  1.0f}, {}, {}, {0xFFFFFFFF}},
		{{ 1.0f,  1.0f, -1.0f}, {}, {}, {0xFFFFFFFF}},
		{{ 1.0f,  1.0f,  1.0f}, {}, {}, {0xFFFFFFFF}},
	};
	v3u BasicCubeFaces[] =
	{
		{0, 2, 1},
		{2, 6, 1},
		{0, 3, 4},
		{0, 4, 2},
		{0, 1, 5},
		{0, 5, 3},
		{2, 4, 7},
		{2, 7, 6},
		{5, 6, 7},
		{5, 1, 6},
		{3, 5, 4},
		{5, 7, 4},
	};

	// TODO: These are taking up indexes that the app could want to use!
	State->Cube = RendererMesh(0, ArrayCount(BasicCubeVertices), ArrayCount(BasicCubeFaces));
	OpenGLLoadMesh(State, BasicCubeVertices, BasicCubeFaces, State->Cube);

	// TODO: These are taking up indexes that the app could want to use!
	u32 WhitePixels[4] =
	{
		0xFFFFFFFF,
		0xFFFFFFFF,
		0xFFFFFFFF,
		0xFFFFFFFF,
	};
	State->WhiteTexture = RendererTexture(0, 2, 2);
	OpenGLLoadTexture(State, WhitePixels, State->WhiteTexture);
	State->WhiteTexture = RendererTexture(0, 1, 1);

	return State;
}

internal GLuint
OpenGLCompileShader(opengl_state *State, char **ShaderSource, u32 ShaderSourceCount, GLenum ShaderType, shader_type Type)
{
	GLuint Result = 0;

	GLuint ShaderHandle = glCreateShader(ShaderType);
	glShaderSource(ShaderHandle, ShaderSourceCount, (const GLchar **)ShaderSource, 0);
	glCompileShader(ShaderHandle);

	GLint Compiled;
	GLchar InfoLog[512];
	glGetShaderiv(ShaderHandle, GL_COMPILE_STATUS, &Compiled);
	if(!Compiled)
	{
		glGetShaderInfoLog(ShaderHandle, ArrayCount(InfoLog), 0, InfoLog);

#if GAME_DEBUG
		char *ShaderTypeName = (ShaderType == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";
		Outf(State->Info->Errors, "Failed to load shader '%s', shader type: %s\n%s",
			WrapZ(Names_shader_type[Type]),
			WrapZ(ShaderTypeName),
			WrapZ(InfoLog));

#endif

		Assert(State->ShadersLoaded);
	}
	else
	{
		Result = ShaderHandle;
	}

	return Result;
}

internal void
OpenGLGetShaderDefines(opengl_state *State, char *Buffer, u32 Length, shader_type Shader)
{
	switch(Shader)
	{
		case Shader_Scene_Quads:
		{
			FormatString(Buffer, Length,
				"#define MAX_LIGHTS %d\n"
				"#define MAX_DECALS %d\n",
				MAX_LIGHTS,
				MAX_DECALS);
		} break;

		case Shader_Scene_Quads_SDF:
		{
			FormatString(Buffer, Length,
				"#define MAX_LIGHTS %d\n"
				"#define MAX_DECALS %d\n"
				"#define SIGNED_DISTANCE_FIELD\n",
				MAX_LIGHTS,
				MAX_DECALS);
		} break;

		case Shader_Scene_Meshes:
		{
			FormatString(Buffer, Length,
				"#define MAX_LIGHTS %d\n"
				"#define MAX_DECALS %d\n"
				"#define MESH_SHADER\n",
				MAX_LIGHTS,
				MAX_DECALS);
		} break;

		case Shader_Overlay_SDF:
		{
			FormatString(Buffer, Length,
				"#define SIGNED_DISTANCE_FIELD\n");
		} break;

		case Shader_ResolveDepthPeel:
		{
			FormatString(Buffer, Length,
				"#define MULTISAMPLE_COUNT %d\n",
				State->Multisamples);
		} break;

		case Shader_CompositeDepthPeel:
		{
			FormatString(Buffer, Length,
				"#define DEPTH_PEEL_COUNT %d\n",
				State->DepthPeelCount);
		} break;
	}
}

internal GLuint
OpenGLLoadCompileAndLinkShaderProgram(opengl_state *State, shader_type Header, shader_type Shader)
{
	GLuint Result = 0;

	temporary_memory TempMem = BeginTemporaryMemory(&State->Region);

	char Defines[1024] = {};
	OpenGLGetShaderDefines(State, Defines, ArrayCount(Defines), Shader);

	char *ShaderSource = 0;
	char *HeaderSource = 0;
	while((ShaderSource == 0) || (HeaderSource == 0))
	{
		// TODO: Infinite loops if we can't open the files.
		ShaderSource = (char *)Platform->LoadEntireFileAndNullTerminate(&State->Region, State->ShaderFilenames[Shader]).Contents;
		HeaderSource = (char *)Platform->LoadEntireFileAndNullTerminate(&State->Region, State->ShaderFilenames[Header]).Contents;
	}

	char *VertexShader[16] = {"#version 330 core\n", "#define VERTEX_SHADER\n",};
	char *FragmentShader[16] = {"#version 330 core\n", "#define FRAGMENT_SHADER\n",};

	u32 Offset = 2;
	VertexShader[Offset] = Defines;
	FragmentShader[Offset] = Defines;

	Assert((Offset + 3) < ArrayCount(VertexShader));
	Assert((Offset + 3) < ArrayCount(FragmentShader));

	VertexShader[Offset + 1] = HeaderSource;
	VertexShader[Offset + 2] = ShaderSource;
	FragmentShader[Offset + 1] = HeaderSource;
	FragmentShader[Offset + 2] = ShaderSource;

	u32 VertexSize = Offset + 3;
	u32 FragmentSize = Offset + 3;

	GLuint VertexShaderHandle = OpenGLCompileShader(State, VertexShader, VertexSize, GL_VERTEX_SHADER, Shader);
	GLuint FragmentShaderHandle = OpenGLCompileShader(State, FragmentShader, FragmentSize, GL_FRAGMENT_SHADER, Shader);

	if((VertexShaderHandle > 0) && (FragmentShaderHandle > 0))
	{
		GLuint ProgramIndex = glCreateProgram();
		glAttachShader(ProgramIndex, VertexShaderHandle);
		glAttachShader(ProgramIndex, FragmentShaderHandle);
		glLinkProgram(ProgramIndex);

		GLint Linked;
		GLchar InfoLog[512];
		glGetProgramiv(ProgramIndex, GL_LINK_STATUS, &Linked);
		if(!Linked)
		{
			glGetProgramInfoLog(ProgramIndex, ArrayCount(InfoLog), 0, InfoLog);
#if GAME_DEBUG
			Outf(State->Info->Errors, "Failed to link shader '%s'\n%s",
				WrapZ(Names_shader_type[Shader]),
				WrapZ(InfoLog));
#endif

			Assert(State->ShadersLoaded);
		}
		else
		{
			Result = ProgramIndex;
		}

	}

	glDeleteShader(VertexShaderHandle);
	glDeleteShader(FragmentShaderHandle);

	EndTemporaryMemory(&TempMem);
	return Result;
}

internal void
OpenGLLinkSamplers(opengl_shader_common *Common,
	char *Sampler0 = 0,
	char *Sampler1 = 0,
	char *Sampler2 = 0,
	char *Sampler3 = 0,
	char *Sampler4 = 0,
	char *Sampler5 = 0,
	char *Sampler6 = 0,
	char *Sampler7 = 0,
	char *Sampler8 = 0,
	char *Sampler9 = 0,
	char *Sampler10 = 0,
	char *Sampler11 = 0,
	char *Sampler12 = 0,
	char *Sampler13 = 0,
	char *Sampler14 = 0,
	char *Sampler15 = 0)
{
    char *SamplerNames[] =
    {
        Sampler0,
        Sampler1,
        Sampler2,
        Sampler3,
        Sampler4,
        Sampler5,
        Sampler6,
        Sampler7,
        Sampler8,
        Sampler9,
        Sampler10,
        Sampler11,
        Sampler12,
        Sampler13,
        Sampler14,
        Sampler15,
    };

    for(u32 SamplerIndex = 0;
        SamplerIndex < ArrayCount(SamplerNames);
        ++SamplerIndex)
    {
        char *Name = SamplerNames[SamplerIndex];
        if(Name)
        {
            GLuint SamplerID = glGetUniformLocation(Common->Handle, Name);
            Assert(Common->SamplerCount < ArrayCount(Common->Samplers));
            Common->Samplers[Common->SamplerCount++] = SamplerID;
            // Assert(SamplerID != -1);
        }
    }
}

internal void
SetUpShaderCommon(opengl_shader_common *Common, GLuint ShaderHandle, shader_type Type)
{
	Common->Type = Type;
	Common->Handle = ShaderHandle;
	Common->Transform = glGetUniformLocation(Common->Handle, "Transform");
}

internal void
OpenGLReloadShader(opengl_state *State, shader_type Type)
{
	GLuint ShaderHandle = OpenGLLoadCompileAndLinkShaderProgram(State, Shader_Header, Type);

	if(ShaderHandle)
	{
		opengl_shader_common *Common = State->Shaders[Type];
		if(Common)
		{
			glDeleteProgram(Common->Handle);
			Common->Handle = 0;
			Common->Transform = -1;

			for(u32 Index = 0;
				Index < Common->SamplerCount;
				++Index)
			{
				Common->Samplers[Index] = 0;
			}

			Common->SamplerCount = 0;
		}

		switch(Type)
		{
			case Shader_Scene_Quads:
			case Shader_Scene_Quads_SDF:
			case Shader_Scene_Meshes:
			{
				shader_scene *Scene = (shader_scene *)State->Shaders[Type];
				if(!Scene)
				{
					Scene = PushStruct(&State->Region, shader_scene);
					State->Shaders[Type] = (opengl_shader_common *)Scene;
				}

				SetUpShaderCommon(&Scene->Common, ShaderHandle, Type);

	            Scene->Projection = glGetUniformLocation(Scene->Common.Handle, "Projection");
				Scene->View = glGetUniformLocation(Scene->Common.Handle, "View");
				Scene->Model = glGetUniformLocation(Scene->Common.Handle, "Model");
				Scene->NormalMatrix = glGetUniformLocation(Scene->Common.Handle, "NormalMatrix");
				Scene->MeshTextureIndex = glGetUniformLocation(Scene->Common.Handle, "MeshTextureIndex");

				Scene->Jitter = glGetUniformLocation(Scene->Common.Handle, "Jitter");
				Scene->FarPlane = glGetUniformLocation(Scene->Common.Handle, "FarPlane");

				Scene->Color = glGetUniformLocation(Scene->Common.Handle, "Color");
				Scene->Roughness = glGetUniformLocation(Scene->Common.Handle, "Roughness");
				Scene->Metalness = glGetUniformLocation(Scene->Common.Handle, "Metalness");

				Scene->LightCount = glGetUniformLocation(Scene->Common.Handle, "LightCount");
				for(u32 LightIndex = 0;
					LightIndex < MAX_LIGHTS;
					++LightIndex)
				{
					char Buffer[256] = {};
					Scene->LightData[LightIndex].IsDirectional = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Lights[%d].IsDirectional", LightIndex).Text);
					Scene->LightData[LightIndex].Color = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Lights[%d].Color", LightIndex).Text);
					Scene->LightData[LightIndex].Intensity = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Lights[%d].Intensity", LightIndex).Text);
					Scene->LightData[LightIndex].Direction = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Lights[%d].Direction", LightIndex).Text);
					Scene->LightData[LightIndex].FuzzFactor = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Lights[%d].FuzzFactor", LightIndex).Text);

					Scene->LightData[LightIndex].P = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Lights[%d].P", LightIndex).Text);
					Scene->LightData[LightIndex].Size = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Lights[%d].Size", LightIndex).Text);
					Scene->LightData[LightIndex].LinearAttenuation = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Lights[%d].LinearAttenuation", LightIndex).Text);
					Scene->LightData[LightIndex].QuadraticAttenuation = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Lights[%d].QuadraticAttenuation", LightIndex).Text);
					Scene->LightData[LightIndex].Radius = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Lights[%d].Radius", LightIndex).Text);
				}

				Scene->DecalCount = glGetUniformLocation(Scene->Common.Handle, "DecalCount");
				for(u32 DecalIndex = 0;
					DecalIndex < MAX_DECALS;
					++DecalIndex)
				{
					char Buffer[256] = {};
					Scene->DecalData[DecalIndex].Projection = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Decals[%d].Projection", DecalIndex).Text);
					Scene->DecalData[DecalIndex].View = glGetUniformLocation(Scene->Common.Handle, FormatString(Buffer, ArrayCount(Buffer), "Decals[%d].View", DecalIndex).Text);
				}

				OpenGLLinkSamplers(&Scene->Common, "TextureArray", "PrevDepthTexture", "DecalTexture");
			} break;

			case Shader_ResolveDepthPeel:
			{
				shader_resolve_depth_peel *Resolve = (shader_resolve_depth_peel *)State->Shaders[Type];
				if(!Resolve)
				{
					Resolve = PushStruct(&State->Region, shader_resolve_depth_peel);
					State->Shaders[Type] = (opengl_shader_common *)Resolve;
				}

				SetUpShaderCommon(&Resolve->Common, ShaderHandle, Type);

	            OpenGLLinkSamplers(&Resolve->Common, "ColorTexture", "DepthTexture");
			} break;

			case Shader_CompositeDepthPeel:
			{
				shader_composite_depth_peel *Composite = (shader_composite_depth_peel *)State->Shaders[Type];
				if(!Composite)
				{
					Composite = PushStruct(&State->Region, shader_composite_depth_peel);
					State->Shaders[Type] = (opengl_shader_common *)Composite;
				}

				SetUpShaderCommon(&Composite->Common, ShaderHandle, Type);

	            Composite->MinPeel = glGetUniformLocation(Composite->Common.Handle, "MinPeel");

	            OpenGLLinkSamplers(&Composite->Common,
	            	"DepthPeels[0]",
	            	"DepthPeels[1]",
	            	"DepthPeels[2]",
	            	"DepthPeels[3]",
	            	"DepthPeels[4]",
	            	"DepthPeels[5]",
	            	"DepthPeels[6]",
	            	"DepthPeels[7]");
			} break;

			case Shader_Overlay:
			case Shader_Overlay_SDF:
			{
				shader_overlay *Overlay = (shader_overlay *)State->Shaders[Type];
				if(!Overlay)
				{
					Overlay = PushStruct(&State->Region, shader_overlay);
					State->Shaders[Type] = (opengl_shader_common *)Overlay;
				}

				SetUpShaderCommon(&Overlay->Common, ShaderHandle, Type);

				OpenGLLinkSamplers(&Overlay->Common, "TextureArray");
			} break;

			case Shader_Overbright:
			{
				shader_overbright *Overbright = (shader_overbright *)State->Shaders[Type];
				if(!Overbright)
				{
					Overbright = PushStruct(&State->Region, shader_overbright);
					State->Shaders[Type] = (opengl_shader_common *)Overbright;
				}

				SetUpShaderCommon(&Overbright->Common, ShaderHandle, Type);

				OpenGLLinkSamplers(&Overbright->Common, "SceneTexture");
			} break;

			case Shader_Blur:
			{
				shader_blur *Blur = (shader_blur *)State->Shaders[Type];
				if(!Blur)
				{
					Blur = PushStruct(&State->Region, shader_blur);
					State->Shaders[Type] = (opengl_shader_common *)Blur;
				}

				SetUpShaderCommon(&Blur->Common, ShaderHandle, Type);

				Blur->Kernel = glGetUniformLocation(Blur->Common.Handle, "Kernel");

				OpenGLLinkSamplers(&Blur->Common, "Texture");
			} break;

			case Shader_Finalize:
			{
				shader_finalize *Finalize = (shader_finalize *)State->Shaders[Type];
				if(!Finalize)
				{
					Finalize = PushStruct(&State->Region, shader_finalize);
					State->Shaders[Type] = (opengl_shader_common *)Finalize;
				}

				SetUpShaderCommon(&Finalize->Common, ShaderHandle, Type);

				OpenGLLinkSamplers(&Finalize->Common, "SceneTexture", "OverlayTexture", "BloomTexture");
			} break;

			case Shader_DebugView:
			{
				shader_debug_view *DebugView = (shader_debug_view *)State->Shaders[Type];
				if(!DebugView)
				{
					DebugView = PushStruct(&State->Region, shader_debug_view);
					State->Shaders[Type] = (opengl_shader_common *)DebugView;
				}

				SetUpShaderCommon(&DebugView->Common, ShaderHandle, Type);

				DebugView->LOD = glGetUniformLocation(DebugView->Common.Handle, "LOD");
				OpenGLLinkSamplers(&DebugView->Common, "Texture");
			} break;

			InvalidDefaultCase;
		}

#if GAME_DEBUG
		Outf(State->Info, "Shader loaded (%s)", WrapZ(Names_shader_type[Type]));
#endif
	}
}

internal void
OpenGLCreateFramebuffer(opengl_framebuffer *Framebuffer, texture_settings *Settings, u32 TextureCount, v2i Resolution)
{
	TIMED_FUNCTION();

	Assert(TextureCount < ArrayCount(Framebuffer->Textures));

	glGenFramebuffers(1, &Framebuffer->Handle);
	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer->Handle);

	Framebuffer->Resolution = Resolution;
	Framebuffer->TextureCount = TextureCount;

	u32 ColorAttachments = 0;
	GLuint Attachments[MAX_FRAMEBUFFER_TEXTURES] = {};

	b32 DepthAttachment = false;
	for(u32 TextureIndex = 0;
		TextureIndex < TextureCount;
		++TextureIndex)
	{
		texture_settings *TextureSettings = Settings + TextureIndex;
		TextureSettings->Width = Resolution.x;
		TextureSettings->Height = Resolution.y;
		Framebuffer->Textures[TextureIndex] = OpenGLAllocateAndLoadTexture(*TextureSettings, 0);
		GLenum TextureTarget = (TextureSettings->Flags & Texture_Multisample) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

		if(TextureSettings->Flags & Texture_DepthBuffer)
		{
			Assert(!DepthAttachment);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				TextureTarget, Framebuffer->Textures[TextureIndex], 0);
			DepthAttachment = true;
		}
		else
		{
			Attachments[ColorAttachments] = GL_COLOR_ATTACHMENT0 + ColorAttachments;
			glFramebufferTexture2D(GL_FRAMEBUFFER, Attachments[ColorAttachments],
				TextureTarget, Framebuffer->Textures[TextureIndex], 0);
			++ColorAttachments;
		}
	}

	glDrawBuffers(ColorAttachments, Attachments);

	Assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

internal void
OpenGLCreateAllFramebuffers(opengl_state *State)
{
	v2i Resolution = State->CurrentRenderSettings.Resolution;
	v2i DownScaleRes = V2ToV2i(State->CurrentRenderSettings.RenderScale*Resolution);

	{
		opengl_framebuffer *SceneBuffer = State->Framebuffers + Framebuffer_Scene;
		texture_settings Settings[] =
		{
			TextureSettings(0, 0, 3, 2, Texture_Float|Texture_ClampToEdge), // Color
		};

		u32 TextureCount = ArrayCount(Settings);
		OpenGLCreateFramebuffer(SceneBuffer, Settings, TextureCount, DownScaleRes);
	}

	{
		State->DepthPeelCount = Clamp(State->CurrentRenderSettings.DesiredDepthPeelCount, 1, ArrayCount(State->DepthPeelsMS));
		State->Multisamples = Clamp(State->CurrentRenderSettings.DesiredMultisamples, 1, 8);

		for(u32 DepthPeelIndex = 0;
			DepthPeelIndex < State->DepthPeelCount;
			++DepthPeelIndex)
		{
			opengl_framebuffer *DepthPeelMS = State->DepthPeelsMS + DepthPeelIndex;
			texture_settings SettingsMS[] =
			{
				TextureSettings(0, 0, 4, 2, Texture_Float|Texture_Multisample, State->Multisamples),
				TextureSettings(0, 0, 1, 4, Texture_Float|Texture_DepthBuffer|Texture_Multisample, State->Multisamples),
			};

			u32 TextureCount = ArrayCount(SettingsMS);
			OpenGLCreateFramebuffer(DepthPeelMS, SettingsMS, TextureCount, DownScaleRes);

			opengl_framebuffer *DepthPeelResolve = State->DepthPeelsResolved + DepthPeelIndex;
			texture_settings SettingsResolve[] =
			{
				TextureSettings(0, 0, 4, 2, Texture_Float|Texture_ClampToEdge),
				TextureSettings(0, 0, 1, 4, Texture_Float|Texture_DepthBuffer|Texture_ClampToBorder, 0, V4(1,1,1,1)),
			};

			TextureCount = ArrayCount(SettingsResolve);
			OpenGLCreateFramebuffer(DepthPeelResolve, SettingsResolve, TextureCount, DownScaleRes);
		}
	}

	if(State->CurrentRenderSettings.Bloom)
	{
		{
			opengl_framebuffer *OverbrightFramebuffer = State->Framebuffers + Framebuffer_Overbright;
			texture_settings Settings[] =
			{
				TextureSettings(0, 0, 3, 2, Texture_Float|Texture_ClampToEdge), // Color
			};

			u32 TextureCount = ArrayCount(Settings);
			OpenGLCreateFramebuffer(OverbrightFramebuffer, Settings, TextureCount, DownScaleRes/2);
		}

		for(u32 Index = 0;
			Index < 2;
			++Index)
		{
			opengl_framebuffer *BlurFramebuffer = State->Framebuffers + Framebuffer_Blur0 + Index;
			texture_settings Settings[] =
			{
				TextureSettings(0, 0, 3, 2, Texture_Float|Texture_ClampToEdge), // Color
			};

			u32 TextureCount = ArrayCount(Settings);
			OpenGLCreateFramebuffer(BlurFramebuffer, Settings, TextureCount, DownScaleRes/2);
		}
	}

	{
		opengl_framebuffer *OverlayFramebuffer = State->Framebuffers + Framebuffer_Overlay;
		texture_settings Settings[] =
		{
			TextureSettings(0, 0, 4, 1, Texture_ClampToEdge), // Color
		};

		u32 TextureCount = ArrayCount(Settings);
		OpenGLCreateFramebuffer(OverlayFramebuffer, Settings, TextureCount, Resolution);
	}
}

internal void
OpenGLFreeFramebuffer(opengl_framebuffer *Framebuffer)
{
	glDeleteTextures(Framebuffer->TextureCount, Framebuffer->Textures);
	glDeleteFramebuffers(1, &Framebuffer->Handle);
	Framebuffer->Handle = 0;
	for(u32 Index = 0;
		Index < Framebuffer->TextureCount;
		++Index)
	{
		Framebuffer->Textures[Index] = 0;
	}

	Framebuffer->TextureCount = 0;
}

internal void
OpenGLChangeSettings(opengl_state *State, render_settings *NewSettings)
{
	TIMED_FUNCTION();

	if(!AreEqual(NewSettings, &State->CurrentRenderSettings))
	{
		render_settings OldSettings = State->CurrentRenderSettings;
		State->CurrentRenderSettings = *NewSettings;
		State->CurrentRenderSettings.SwapInterval = OldSettings.SwapInterval; // NOTE: This is set closer to the platform layer.

		// TODO: We don't have to destroy and recreate everything on
		// any change to render_settings.

		for(u32 FramebufferIndex = 1;
			FramebufferIndex < Framebuffer_Count;
			++FramebufferIndex)
		{
			opengl_framebuffer *Framebuffer = State->Framebuffers + FramebufferIndex;
			OpenGLFreeFramebuffer(Framebuffer);
		}

		// TODO: Depending on the settings, there are framebuffers and
		// shaders we don't have to load.
		OpenGLCreateAllFramebuffers(State);

		for(u32 ShaderIndex = Shader_Header + 1;
			ShaderIndex < Shader_Count;
			++ShaderIndex)
		{
			OpenGLReloadShader(State, (shader_type) ShaderIndex);
		}
		State->ShadersLoaded = true;
	}
}

inline void
DrawMesh(opengl_state *State, u32 MeshIndex)
{
	opengl_mesh_info *MeshInfo = State->Meshes + MeshIndex;
	glBindVertexArray(MeshInfo->VAO);
	glDrawElements(GL_TRIANGLES, 3*MeshInfo->FaceCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

inline void
DrawCube(opengl_state *State)
{
	DrawMesh(State, State->Cube.Index);
}

inline void
DrawQuad(opengl_state *State)
{
	DrawMesh(State, State->Quad.Index);
}

inline void
SetUniform(GLint UniformLocation, b32 Boolean)
{
	glUniform1i(UniformLocation, Boolean);
}

inline void
SetUniform(GLint UniformLocation, u32 UInt)
{
	glUniform1i(UniformLocation, UInt);
}

inline void
SetUniform(GLint UniformLocation, r32 Value)
{
	glUniform1f(UniformLocation, Value);
}

inline void
SetUniform(GLint UniformLocation, mat3 Matrix3)
{
	glUniformMatrix3fv(UniformLocation, 1, GL_FALSE, Matrix3.M);
}

inline void
SetUniform(GLint UniformLocation, mat4 Matrix4)
{
	glUniformMatrix4fv(UniformLocation, 1, GL_FALSE, Matrix4.M);
}

inline void
SetUniform(GLint UniformLocation, v2 Vector2)
{
	glUniform2f(UniformLocation, Vector2.x, Vector2.y);
}

inline void
SetUniform(GLint UniformLocation, v2i Vector2i)
{
	glUniform2i(UniformLocation, Vector2i.x, Vector2i.y);
}

inline void
SetUniform(GLint UniformLocation, v3 Vector3)
{
	glUniform3fv(UniformLocation, 1, Vector3.E);
}

inline void
SetUniform(GLint UniformLocation, v4 Vector4)
{
	glUniform4fv(UniformLocation, 1, Vector4.E);
}

inline void
BindTexture(GLenum Slot, GLint TextureHandle, GLenum Target = GL_TEXTURE_2D)
{
	glActiveTexture(Slot);
	glBindTexture(Target, TextureHandle);
}

internal void
OpenGLBindFramebuffer(opengl_framebuffer *Framebuffer, u32 Width = 0, u32 Height = 0)
{
	TIMED_FUNCTION();

	if(Framebuffer)
	{
		if(!Width)
		{
			Width = Framebuffer->Resolution.x;
		}
		if(!Height)
		{
			Height = Framebuffer->Resolution.y;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer->Handle);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glViewport(0, 0, Width, Height);
	glScissor(0, 0, Width, Height);
}

internal void
NextJitter(opengl_state *State)
{
	++State->JitterIndex;

	v2 Halton23[] =
	{
		V2(1.0f/2.0f, 1.0f/3.0f),
		V2(1.0f/4.0f, 2.0f/3.0f),
		V2(3.0f/4.0f, 1.0f/9.0f),
		V2(1.0f/8.0f, 4.0f/9.0f),
		V2(5.0f/8.0f, 7.0f/9.0f),
		V2(3.0f/8.0f, 2.0f/9.0f),
		V2(7.0f/8.0f, 5.0f/9.0f),
		V2(1.0f/16.0f, 8.0f/9.0f),
		V2(9.0f/16.0f, 1.0f/27.0f),
	};
	v2 Jitter = Halton23[State->JitterIndex % ArrayCount(Halton23)];

	Jitter = 2.0f*Jitter - V2(1,1);
	Jitter.x /= State->CurrentRenderSettings.Resolution.x;
	Jitter.y /= State->CurrentRenderSettings.Resolution.x;
	State->Jitter = Jitter;
}

internal void
SetLightUniforms(opengl_state *State, shader_scene *SceneShader,
	render_setup *Setup, game_render_commands *RenderCommands)
{
	SetUniform(SceneShader->LightCount, RenderCommands->LightCount);

	for(u32 LightIndex = 0;
		LightIndex < RenderCommands->LightCount;
		++LightIndex)
	{
		light *Light = RenderCommands->Lights + LightIndex;
		switch(Light->Type)
		{
			case LightType_Point:
			{
				point_light *PointLight = &Light->Point;
				v3 LightP = (Setup->View * V4(PointLight->P, 1.0f)).xyz;

				SetUniform(SceneShader->LightData[LightIndex].IsDirectional, false);
				SetUniform(SceneShader->LightData[LightIndex].Color, PointLight->Color);
				SetUniform(SceneShader->LightData[LightIndex].Intensity, PointLight->Intensity);
				SetUniform(SceneShader->LightData[LightIndex].P, LightP);
				SetUniform(SceneShader->LightData[LightIndex].Size, PointLight->Size);
				SetUniform(SceneShader->LightData[LightIndex].LinearAttenuation, PointLight->LinearAttenuation);
				SetUniform(SceneShader->LightData[LightIndex].QuadraticAttenuation, PointLight->QuadraticAttenuation);
				SetUniform(SceneShader->LightData[LightIndex].Radius, PointLight->Radius);
			} break;

			case LightType_Directional:
			{
				directional_light *DirectionalLight = &Light->Directional;
				v3 LightDirection = (Setup->View * V4(DirectionalLight->Direction, 0.0f)).xyz;
				f32 FuzzFactor = Tan(0.5f*DirectionalLight->ArcDegrees*Pi32/180.0f);

				SetUniform(SceneShader->LightData[LightIndex].IsDirectional, true);
				SetUniform(SceneShader->LightData[LightIndex].Color, DirectionalLight->Color);
				SetUniform(SceneShader->LightData[LightIndex].Intensity, DirectionalLight->Intensity);
				SetUniform(SceneShader->LightData[LightIndex].Direction, LightDirection);
				SetUniform(SceneShader->LightData[LightIndex].FuzzFactor, FuzzFactor);
			} break;

			case LightType_Ambient:
			{
				ambient_light *AmbientLight = &Light->Ambient;

				SetUniform(SceneShader->LightData[LightIndex].IsDirectional, true);
				SetUniform(SceneShader->LightData[LightIndex].Color, AmbientLight->Color);
				SetUniform(SceneShader->LightData[LightIndex].Direction, V3(0,0,0));
				SetUniform(SceneShader->LightData[LightIndex].FuzzFactor, AmbientLight->Coeff);
			} break;

			InvalidDefaultCase;
		}
	}
}

internal void
SetDecalUniforms(opengl_state *State, shader_scene *SceneShader, game_render_commands *RenderCommands)
{
	SetUniform(SceneShader->DecalCount, RenderCommands->DecalCount);

	for(u32 Index = 0;
		Index < RenderCommands->DecalCount;
		++Index)
	{
		decal *Decal = RenderCommands->Decals + Index;
		BindTexture(GL_TEXTURE2, (GLuint) Decal->TextureHandle.Index);
		mat4 DecalProjection = OrthogonalProjectionMat4(0.5f*Decal->Box.Dim.x, 0.5f*Decal->Box.Dim.y,
			-0.5f*Decal->Box.Dim.z, 0.5f*Decal->Box.Dim.z);
		mat4 DecalView = RotationMat4(Conjugate(Decal->Box.Rotation))*TranslationMat4(-Decal->Box.P);
		SetUniform(SceneShader->DecalData[Index].Projection, DecalProjection);
		SetUniform(SceneShader->DecalData[Index].View, DecalView);
	}
}

internal void
SetSceneShaderUniforms(opengl_state *State, shader_scene *Scene,
	render_setup *Setup, game_render_commands *RenderCommands)
{
	SetLightUniforms(State, Scene, Setup, RenderCommands);
	SetDecalUniforms(State, Scene, RenderCommands);

	SetUniform(Scene->Projection, Setup->Projection);
	SetUniform(Scene->View, Setup->View);
	SetUniform(Scene->FarPlane, Setup->FarPlane);
	SetUniform(Scene->Jitter, State->Jitter);

	GLuint PrevPeelDepthTexture = 0;
	if(State->CurrentDepthPeel > 0)
	{
		PrevPeelDepthTexture = State->DepthPeelsResolved[State->CurrentDepthPeel - 1].Textures[FBT_Depth];
	}

	BindTexture(GL_TEXTURE1, PrevPeelDepthTexture);
}

internal opengl_shader_common *
BeginShader(opengl_state *State, shader_type Type)
{
	TIMED_FUNCTION();

	opengl_shader_common *Shader = State->Shaders[Type];

	Assert(Shader);
	glUseProgram(Shader->Handle);
	State->CurrentShader = Shader;

	for(u32 SamplerIndex = 0;
		SamplerIndex < Shader->SamplerCount;
		++SamplerIndex)
	{
		SetUniform(Shader->Samplers[SamplerIndex], SamplerIndex);
	}

	return Shader;
}

internal void
EndShader(opengl_state *State)
{
	glUseProgram(0);
	State->CurrentShader = 0;
}

internal void
OpenGLRenderSetup(opengl_state *State, render_setup *Setup)
{
	u32 Width = Setup->ClipRectMax.x - Setup->ClipRectMin.x;
	u32 Height = Setup->ClipRectMax.y - Setup->ClipRectMin.y;
	glScissor(Setup->ClipRectMin.x, Setup->ClipRectMin.y,
		Width, Height);

	if(Setup->Flags & RenderFlag_TopMost)
	{
		glDisable(GL_DEPTH_TEST);
		// glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}

	if(Setup->Flags & RenderFlag_AlphaBlended)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

internal void
OpenGLRenderQuad(opengl_state *State, v2 Min, v2 Max)
{
	v2 Scale = Max - Min;
	mat4 Transform =
	{
		2.0f*Scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f*Scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		2.0f*Min.x - 1.0f, 2.0f*Min.y - 1.0f, 0.0f, 1.0f,
	};

	SetUniform(State->CurrentShader->Transform, Transform);
	DrawQuad(State);
}

internal void
OpenGLExecuteRenderCommands(opengl_state *State, game_render_commands *RenderCommands)
{
	TIMED_FUNCTION();

	glBindBuffer(GL_ARRAY_BUFFER, State->ArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER,
		RenderCommands->VertexCount*sizeof(vertex_format),
		RenderCommands->VertexArray,
		GL_STREAM_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, State->IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		RenderCommands->IndexCount*sizeof(RenderCommands->IndexArray[0]),
		RenderCommands->IndexArray,
		GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	u32 BytesRead = 0;
	u32 DepthPeelBeginSnapshot = 0;
	while(BytesRead < RenderCommands->Size)
	{
		render_entry_header *Header = (render_entry_header *)(RenderCommands->Buffer + BytesRead);
		BytesRead += sizeof(render_entry_header);
		switch(Header->Type)
		{
			case RenderCommandType_render_entry_set_target:
			{
				render_entry_set_target *Entry = (render_entry_set_target *)(Header + 1);
				BytesRead += sizeof(render_entry_set_target);

				switch(Entry->Target)
				{
					case Target_Scene:
					{
						OpenGLBindFramebuffer(State->DepthPeelsMS + State->CurrentDepthPeel);
					} break;

					case Target_Overlay:
					{
						OpenGLBindFramebuffer(State->Framebuffers + Framebuffer_Overlay);
					} break;

					InvalidDefaultCase;
				}
			} break;

			case RenderCommandType_render_entry_full_clear:
			{
				render_entry_full_clear *Entry = (render_entry_full_clear *)(Header + 1);
				BytesRead += sizeof(render_entry_full_clear);

				f32 MaxDepth = 1.0f;
				glClearBufferfv(GL_COLOR, 0, Entry->Color.E);
				glClearBufferfv(GL_DEPTH, 0, &MaxDepth);
			} break;

			case RenderCommandType_render_entry_clear_depth:
			{
				f32 MaxDepth = 1.0f;
				glClearBufferfv(GL_DEPTH, 0, &MaxDepth);
			} break;

			case RenderCommandType_render_entry_begin_depth_peels:
			{
				Assert(State->CurrentDepthPeel < State->DepthPeelCount);
				DepthPeelBeginSnapshot = (BytesRead - sizeof(render_entry_header));
				glDepthFunc(GL_LEQUAL);
			} break;

			case RenderCommandType_render_entry_end_depth_peels:
			{
				opengl_framebuffer *MSBuffer = State->DepthPeelsMS + State->CurrentDepthPeel;
				opengl_framebuffer *ResolvedBuffer = State->DepthPeelsResolved + State->CurrentDepthPeel;
				OpenGLBindFramebuffer(ResolvedBuffer);

				shader_resolve_depth_peel *ResolveDepthPeel = (shader_resolve_depth_peel *) BeginShader(State, Shader_ResolveDepthPeel);
				glEnable(GL_DEPTH_TEST);
				glDisable(GL_BLEND);
				glDepthFunc(GL_ALWAYS);
				BindTexture(GL_TEXTURE0, MSBuffer->Textures[FBT_Color], GL_TEXTURE_2D_MULTISAMPLE);
				BindTexture(GL_TEXTURE1, MSBuffer->Textures[FBT_Depth], GL_TEXTURE_2D_MULTISAMPLE);
				OpenGLRenderQuad(State, V2(0,0), V2(1,1));
				BindTexture(GL_TEXTURE0, 0, GL_TEXTURE_2D_MULTISAMPLE);
				BindTexture(GL_TEXTURE1, 0, GL_TEXTURE_2D_MULTISAMPLE);
				EndShader(State);
				glDisable(GL_DEPTH_TEST);

				if(++State->CurrentDepthPeel == State->DepthPeelCount)
				{
					// NOTE: Composite depth peels.
					OpenGLBindFramebuffer(State->Framebuffers + Framebuffer_Scene);
					shader_composite_depth_peel *Composite = (shader_composite_depth_peel *) BeginShader(State, Shader_CompositeDepthPeel);
					SetUniform(Composite->MinPeel, State->DEBUGDisplayPeel);

					for(u32 DepthPeelIndex = 0;
						DepthPeelIndex < State->DepthPeelCount;
						++DepthPeelIndex)
					{
						opengl_framebuffer *DepthPeel = State->DepthPeelsResolved + DepthPeelIndex;
						BindTexture(GL_TEXTURE0 + DepthPeelIndex, DepthPeel->Textures[FBT_Color]);
					}

					OpenGLRenderQuad(State, V2(0,0), V2(1,1));

					for(u32 DepthPeelIndex = 0;
						DepthPeelIndex < State->DepthPeelCount;
						++DepthPeelIndex)
					{
						BindTexture(GL_TEXTURE0 + DepthPeelIndex, 0);
					}

					EndShader(State);
				}
				else
				{
					BytesRead = DepthPeelBeginSnapshot;
				}
			} break;

			case RenderCommandType_render_entry_meshes:
			{
				TIMED_BLOCK("OpenGL Render Meshes");

				render_entry_meshes *Entry = (render_entry_meshes *)(Header + 1);
				render_setup *Setup = &Entry->Setup;
				OpenGLRenderSetup(State, Setup);

				Assert(Setup->Target == Target_Scene);
				Assert(!(Setup->Flags & RenderFlag_SDF));
				shader_scene *Scene = (shader_scene *) BeginShader(State, Shader_Scene_Meshes);
				SetSceneShaderUniforms(State, Scene, Setup, RenderCommands);
				BindTexture(GL_TEXTURE0, State->TextureArray, GL_TEXTURE_2D_ARRAY);

				if(State->Wireframes)
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}

				for(u32 MeshIndex = 0;
					MeshIndex < Entry->MeshCount;
					++MeshIndex)
				{
					render_command_mesh_data *MeshData = RenderCommands->MeshArray + Entry->MeshArrayOffset + MeshIndex;

					SetUniform(Scene->Model, MeshData->Model);
					SetUniform(Scene->NormalMatrix, MeshData->NormalMatrix);
					SetUniform(Scene->Roughness, MeshData->Material.Roughness);
					SetUniform(Scene->Metalness, MeshData->Material.Metalness);
					SetUniform(Scene->Color, MeshData->Material.Color);
					SetUniform(Scene->MeshTextureIndex, MeshData->TextureIndex);

					DrawMesh(State, MeshData->MeshIndex);
				}

				BindTexture(GL_TEXTURE0, 0, GL_TEXTURE_2D_ARRAY);

				if(State->Wireframes)
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}

				BytesRead += sizeof(render_entry_meshes);
			} break;

			case RenderCommandType_render_entry_quads:
			{
				TIMED_BLOCK("OpenGL Render Quads");

				render_entry_quads *Entry = (render_entry_quads *)(Header + 1);
				render_setup *Setup = &Entry->Setup;
				OpenGLRenderSetup(State, Setup);

				if(Setup->Target == Target_Scene)
				{
					shader_scene *Scene = 0;
					if(Setup->Flags & RenderFlag_SDF)
					{
						Scene = (shader_scene *) BeginShader(State, Shader_Scene_Quads_SDF);
					}
					else
					{
						Scene = (shader_scene *) BeginShader(State, Shader_Scene_Quads);
						SetUniform(Scene->Color, V4(1,1,1,1));
					}

					SetSceneShaderUniforms(State, Scene, Setup, RenderCommands);
				}
				else
				{
					Assert(Setup->Target == Target_Overlay);
					shader_overlay *Overlay = 0;
					if(Setup->Flags & RenderFlag_SDF)
					{
						Overlay = (shader_overlay *) BeginShader(State, Shader_Overlay_SDF);
					}
					else
					{
						Overlay = (shader_overlay *) BeginShader(State, Shader_Overlay);
					}

					SetUniform(Overlay->Common.Transform, Setup->Projection*Setup->View);
				}

				glBindVertexArray(State->VertexArray);
				// glBindBuffer(GL_ARRAY_BUFFER, State->ArrayBuffer);
				// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, State->IndexBuffer);

				BindTexture(GL_TEXTURE0, State->TextureArray, GL_TEXTURE_2D_ARRAY);
				glDrawElements(GL_TRIANGLES, 6*Entry->QuadCount, GL_UNSIGNED_INT, (GLvoid *)(Entry->IndexArrayOffset*sizeof(u32)));

				BindTexture(GL_TEXTURE0, 0, GL_TEXTURE_2D_ARRAY);
				glBindVertexArray(0);

				BytesRead += sizeof(render_entry_quads);
			} break;

			InvalidDefaultCase;
		}
	}

}

internal void
OpenGLMemoryOp(opengl_state *State, render_memory_op *Op)
{
	switch(Op->Type)
	{
		case RenderOp_LoadMesh:
		{
			load_mesh_op *LoadMeshOp = &Op->LoadMesh;
			renderer_mesh Mesh = LoadMeshOp->Mesh;
			OpenGLLoadMesh(State, (vertex_format *)LoadMeshOp->VertexMemory,
				(v3u *)LoadMeshOp->FaceMemory, Mesh);
		} break;

		case RenderOp_LoadTexture:
		{
			load_texture_op *LoadTextureOp = &Op->LoadTexture;
			renderer_texture Texture = LoadTextureOp->Texture;
			OpenGLLoadTexture(State, LoadTextureOp->Pixels, Texture);
		} break;

		InvalidDefaultCase;
	}
}

internal void
OpenGLProcessMemoryOpQueue(opengl_state *State, render_memory_op_queue *RenderOpQueue)
{
	BeginTicketMutex(&RenderOpQueue->Mutex);
	render_memory_op *FirstRenderOp = RenderOpQueue->First;
	render_memory_op *LastRenderOp = RenderOpQueue->Last;
	RenderOpQueue->First = RenderOpQueue->Last = 0;
	EndTicketMutex(&RenderOpQueue->Mutex);

	if(FirstRenderOp)
	{
		for(render_memory_op *RenderOp = FirstRenderOp;
			RenderOp;
			RenderOp = RenderOp->Next)
		{
			OpenGLMemoryOp(State, RenderOp);
		}

		BeginTicketMutex(&RenderOpQueue->Mutex);
		LastRenderOp->Next = RenderOpQueue->FirstFree;
		RenderOpQueue->FirstFree = FirstRenderOp;
		EndTicketMutex(&RenderOpQueue->Mutex);
	}
}

internal void
OpenGLBeginFrame(opengl_state *State, render_settings *RenderSettings)
{
	OpenGLChangeSettings(State, RenderSettings);
}

internal void
OpenGLEndFrame(opengl_state *State)
{
	TIMED_FUNCTION();

	{DEBUG_DATA_BLOCK("OpenGL Renderer");
		DEBUG_VALUE(&State->Wireframes);
	}

	CheckMemory(&State->Region);

	// GLuint BlueNoise = 0;
	// if(State->BlueNoise)
	// {
	// 	BlueNoise = (GLuint)(umm)State->BlueNoise->Data_;
	// }

	NextJitter(State);

	opengl_framebuffer *SceneBuffer = State->Framebuffers + Framebuffer_Scene;
	opengl_framebuffer *OverlayFramebuffer = State->Framebuffers + Framebuffer_Overlay;
	opengl_framebuffer *OverbrightFramebuffer = State->Framebuffers + Framebuffer_Overbright;

	{DEBUG_DATA_BLOCK("Memory");
		memory_region *OpenGLRegion = &State->Region;
		DEBUG_VALUE(OpenGLRegion);
	}
	{DEBUG_DATA_BLOCK("OpenGL Renderer");
		{DEBUG_DATA_BLOCK("Framebuffers");
			{DEBUG_DATA_BLOCK("Scene");
				DEBUG_UI_ELEMENT_VALUE(Event_Texture, "Color", &SceneBuffer->Textures[FBT_Color]);
			}
			{DEBUG_DATA_BLOCK("Overbright");
				DEBUG_UI_ELEMENT_VALUE(Event_Texture, "Overbright", &OverbrightFramebuffer->Textures[FBT_Color]);
			}
			if(State->DepthPeelCount > 0)
			{DEBUG_DATA_BLOCK("Peel 0");
				opengl_framebuffer *Peel0 = State->DepthPeelsResolved + 0;
				DEBUG_UI_ELEMENT_VALUE(Event_Texture, "Color", &Peel0->Textures[FBT_Color]);
				DEBUG_UI_ELEMENT_VALUE(Event_Texture, "Depth", &Peel0->Textures[FBT_Depth]);
			}
			if(State->DepthPeelCount > 1)
			{DEBUG_DATA_BLOCK("Peel 1");
				opengl_framebuffer *Peel1 = State->DepthPeelsResolved + 1;
				DEBUG_UI_ELEMENT_VALUE(Event_Texture, "Color", &Peel1->Textures[FBT_Color]);
				DEBUG_UI_ELEMENT_VALUE(Event_Texture, "Depth", &Peel1->Textures[FBT_Depth]);
			}
			if(State->DepthPeelCount > 2)
			{DEBUG_DATA_BLOCK("Peel 2");
				opengl_framebuffer *Peel2 = State->DepthPeelsResolved + 2;
				DEBUG_UI_ELEMENT_VALUE(Event_Texture, "Color", &Peel2->Textures[FBT_Color]);
				DEBUG_UI_ELEMENT_VALUE(Event_Texture, "Depth", &Peel2->Textures[FBT_Depth]);
			}
			if(State->DepthPeelCount > 3)
			{DEBUG_DATA_BLOCK("Peel 3");
				opengl_framebuffer *Peel3 = State->DepthPeelsResolved + 3;
				DEBUG_UI_ELEMENT_VALUE(Event_Texture, "Color", &Peel3->Textures[FBT_Color]);
				DEBUG_UI_ELEMENT_VALUE(Event_Texture, "Depth", &Peel3->Textures[FBT_Depth]);
			}
		}
	}

#if 0
	if(Input->KeyStates[Key_F4].WasPressed)
	{
		State->DEBUGDisplayPeel -= 1;
	}
	if(Input->KeyStates[Key_F5].WasPressed)
	{
		State->DEBUGDisplayPeel += 1;
	}
#endif
	State->DEBUGDisplayPeel = Clamp(State->DEBUGDisplayPeel, 0, State->DepthPeelCount - 1);

	{
		// NOTE: Execute all render commands.
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCullFace(GL_BACK);
		glEnable(GL_SCISSOR_TEST);

		OpenGLBindFramebuffer(SceneBuffer);
		f32 MinDepth = 0.0f;
		glClearBufferfv(GL_DEPTH, 0, &MinDepth);

		State->CurrentDepthPeel = 0;
		OpenGLExecuteRenderCommands(State, &State->RenderCommands);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_SCISSOR_TEST);
	}

	opengl_framebuffer *BloomFramebuffer = 0;
	if(State->CurrentRenderSettings.Bloom)
	{
		// NOTE: Overbright/bloom.
		OpenGLBindFramebuffer(OverbrightFramebuffer);
		shader_overbright *Overbright = (shader_overbright *) BeginShader(State, Shader_Overbright);
		BindTexture(GL_TEXTURE0, SceneBuffer->Textures[FBT_Color]);
		OpenGLRenderQuad(State, V2(0,0), V2(1,1));
		BindTexture(GL_TEXTURE0, 0);

		shader_blur *Blur = (shader_blur *) BeginShader(State, Shader_Blur);
		opengl_framebuffer *BlurDest = State->Framebuffers + Framebuffer_Blur0;
		opengl_framebuffer *BlurSource = OverbrightFramebuffer;
		s32 Kernels[] = {0, 1, 2, 2, 3};

		for(u32 Index = 0;
			Index < ArrayCount(Kernels);
			++Index)
		{
			OpenGLBindFramebuffer(BlurDest);
			BindTexture(GL_TEXTURE0, BlurSource->Textures[FBT_Color]);
			SetUniform(Blur->Kernel, Kernels[Index]);
			OpenGLRenderQuad(State, V2(0,0), V2(1,1));
			BindTexture(GL_TEXTURE0, 0);

			BloomFramebuffer = BlurDest;
			BlurSource = State->Framebuffers + Framebuffer_Blur0 + (Index % 2);
			BlurDest = State->Framebuffers + Framebuffer_Blur0 + ((Index + 1) % 2);
		}
	}

	{
		// NOTE: Composite to screen.
		OpenGLBindFramebuffer(0, State->CurrentRenderSettings.Resolution.x, State->CurrentRenderSettings.Resolution.y);
		shader_finalize *Finalize = (shader_finalize *) BeginShader(State, Shader_Finalize);
		glClearBufferfv(GL_COLOR, 0, V4(0,0,0,0).E);

		BindTexture(GL_TEXTURE0, SceneBuffer->Textures[FBT_Color]);
		BindTexture(GL_TEXTURE1, OverlayFramebuffer->Textures[FBT_Color]);
		BindTexture(GL_TEXTURE2, BloomFramebuffer ? BloomFramebuffer->Textures[FBT_Color] : 0);

		OpenGLRenderQuad(State, V2(0,0), V2(1,1));

		BindTexture(GL_TEXTURE0, 0);
		BindTexture(GL_TEXTURE1, 0);
		BindTexture(GL_TEXTURE2, 0);
		EndShader(State);
	}
}
