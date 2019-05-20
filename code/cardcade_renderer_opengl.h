/*@H
* File: cardcade_renderer_opengl.h
* Author: Jesse Calvert
* Created: December 18, 2017, 17:08
* Last modified: November 4, 2018, 18:24
*/

#pragma once

#define GL_SHADING_LANGUAGE_VERSION      	 	0x8B8C

#define GL_FRAMEBUFFER_SRGB               		0x8DB9
#define GL_SRGB8_ALPHA8                   		0x8C43

#define GL_ARRAY_BUFFER                   0x8892

#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_COMPILE_STATUS                 0x8B81
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_LINK_STATUS                    0x8B82

#define GL_ELEMENT_ARRAY_BUFFER           0x8893

#define GL_BGRA                           0x80E1
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_CONSTANT_ALPHA                 0x8003
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_CLAMP_TO_BORDER                0x812D
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7

#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF

#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7
#define GL_COLOR_ATTACHMENT8              0x8CE8
#define GL_COLOR_ATTACHMENT9              0x8CE9
#define GL_COLOR_ATTACHMENT10             0x8CEA
#define GL_COLOR_ATTACHMENT11             0x8CEB
#define GL_COLOR_ATTACHMENT12             0x8CEC
#define GL_COLOR_ATTACHMENT13             0x8CED
#define GL_COLOR_ATTACHMENT14             0x8CEE
#define GL_COLOR_ATTACHMENT15             0x8CEF
#define GL_COLOR_ATTACHMENT16             0x8CF0
#define GL_COLOR_ATTACHMENT17             0x8CF1
#define GL_COLOR_ATTACHMENT18             0x8CF2
#define GL_COLOR_ATTACHMENT19             0x8CF3
#define GL_COLOR_ATTACHMENT20             0x8CF4
#define GL_COLOR_ATTACHMENT21             0x8CF5
#define GL_COLOR_ATTACHMENT22             0x8CF6
#define GL_COLOR_ATTACHMENT23             0x8CF7
#define GL_COLOR_ATTACHMENT24             0x8CF8
#define GL_COLOR_ATTACHMENT25             0x8CF9
#define GL_COLOR_ATTACHMENT26             0x8CFA
#define GL_COLOR_ATTACHMENT27             0x8CFB
#define GL_COLOR_ATTACHMENT28             0x8CFC
#define GL_COLOR_ATTACHMENT29             0x8CFD
#define GL_COLOR_ATTACHMENT30             0x8CFE
#define GL_COLOR_ATTACHMENT31             0x8CFF

#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9

#define GL_RG                             0x8227
#define GL_RG_INTEGER                     0x8228
#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_RG8                            0x822B
#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#define GL_R8I                            0x8231
#define GL_R8UI                           0x8232
#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
#define GL_R32I                           0x8235
#define GL_R32UI                          0x8236
#define GL_RG8I                           0x8237
#define GL_RG8UI                          0x8238
#define GL_RG16I                          0x8239
#define GL_RG16UI                         0x823A
#define GL_RG32I                          0x823B
#define GL_RG32UI                         0x823C
#define GL_SAMPLES_PASSED                 0x8914
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867
#define GL_QUERY_RESULT_NO_WAIT           0x9194

#define GL_DEPTH_COMPONENT32F             0x8CAC
#define GL_DEPTH32F_STENCIL8              0x8CAD
#define GL_DEPTH_STENCIL                  0x84F9
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD

#define GL_STENCIL_INDEX1                 0x8D46
#define GL_STENCIL_INDEX4                 0x8D47
#define GL_STENCIL_INDEX8                 0x8D48
#define GL_STENCIL_INDEX16                0x8D49

#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D

#define GL_MULTISAMPLE                    0x809D
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_TEXTURE_2D_ARRAY               0x8C1A

#define GL_DEBUG_OUTPUT                   0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148

struct opengl_info
{
	char *Vendor;
	char *Renderer;
	char *Version;
	char *ShadingLanguageVersion;
	char *Extensions;

	b32 GL_EXT_texture_sRGB;
	b32 GL_EXT_framebuffer_sRGB;

	GLint MaxSamplersPerShader;
};

struct opengl_shader_common
{
	shader_type Type;
	GLuint Handle;

	GLint Transform;

	u32 SamplerCount;
	GLint Samplers[32];
};

struct light_shader_data
{
	GLint IsDirectional;
	GLint Color;
	GLint Intensity;

	GLint Direction;
	GLint FuzzFactor;
	GLint P;
	GLint Size;
	GLint LinearAttenuation;
	GLint QuadraticAttenuation;
	GLint Radius;
};

struct decal_shader_data
{
	GLint Projection;
	GLint View;
};

struct shader_scene
{
	opengl_shader_common Common;

	GLint Projection;
	GLint View;
	GLint Model;

	GLint NormalMatrix;
	GLint MeshTextureIndex;

	GLint Jitter;
	GLint FarPlane;

	GLint Roughness;
	GLint Metalness;
	GLint Color;

	GLint LightCount;
	light_shader_data LightData[16];

	GLint DecalCount;
	decal_shader_data DecalData[16];
};

struct shader_resolve_depth_peel
{
	opengl_shader_common Common;
};

struct shader_composite_depth_peel
{
	opengl_shader_common Common;

	GLint MinPeel;
};

struct shader_overlay
{
	opengl_shader_common Common;
};

struct shader_overbright
{
	opengl_shader_common Common;
};

struct shader_blur
{
	opengl_shader_common Common;

	GLint Kernel;
};

struct shader_finalize
{
	opengl_shader_common Common;
};

struct shader_debug_view
{
	opengl_shader_common Common;

	GLint LOD;
};

enum texture_flags
{
	Texture_FilterNearest = 0x1,
	Texture_ClampToBorder = 0x2,
	Texture_ClampToEdge = 0x4,
	Texture_Float = 0x8,
	Texture_DepthBuffer = 0x10,
	Texture_Mipmap = 0x20,
	Texture_Multisample = 0x40,
};
struct texture_settings
{
	flag32(texture_flags) Flags;
	u32 Channels;
	u32 BytesPerChannel;
	u32 Samples;
	v4 BorderColor;
	u32 Width;
	u32 Height;
};

INTROSPECT()
enum framebuffer_type
{
	Framebuffer_Default,
	Framebuffer_Scene,
	Framebuffer_Overbright,
	Framebuffer_Overlay,
	Framebuffer_Blur0,
	Framebuffer_Blur1,

	Framebuffer_Count,
};

enum framebuffer_texture_names_0
{
	FBT_Color,
	FBT_Depth,
};

#define MAX_FRAMEBUFFER_TEXTURES 16
#define MAX_DEPTH_PEELS 16

struct opengl_framebuffer
{
	GLuint Handle;
	v2i Resolution;

	u32 TextureCount;
	GLuint Textures[MAX_FRAMEBUFFER_TEXTURES];
};

struct opengl_mesh_info
{
	renderer_mesh Mesh;

	u32 FaceCount;
	u32 VertexCount;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
};

struct opengl_state
{
	platform_renderer PlatformRenderer;
	render_settings CurrentRenderSettings;
	game_render_commands RenderCommands;

	memory_region Region;
	opengl_info OpenGLInfo;
	struct stream *Info;

	opengl_shader_common *CurrentShader;
	opengl_shader_common *Shaders[Shader_Count];

	opengl_framebuffer Framebuffers[Framebuffer_Count];
	u32 CurrentDepthPeel;
	u32 DepthPeelCount;
	u32 Multisamples;
	opengl_framebuffer DepthPeelsMS[MAX_DEPTH_PEELS];
	opengl_framebuffer DepthPeelsResolved[MAX_DEPTH_PEELS];

	GLuint VertexArray;
	GLuint ArrayBuffer;
	GLuint IndexBuffer;

	u32 JitterIndex;
	v2 Jitter;

	// texture *BlueNoise;
	renderer_texture WhiteTexture;
	renderer_mesh Quad;
	renderer_mesh Cube;

	b32 Wireframes;
	u32 DEBUGDisplayPeel;

	b32 ShadersLoaded;
	string ShaderFilenames[Shader_Count];

	u32 MaxTextureCount;
	GLuint TextureArray;
	u32 MaxMeshCount;
	opengl_mesh_info *Meshes;
};

inline b32
OpenGLUniformLocationIsValid(GLint UniformLoc)
{
	b32 Result = (UniformLoc != -1);
	return Result;
}
