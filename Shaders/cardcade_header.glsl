#line 2
#define PI 3.14159265359

#define b32 bool
#define f32 float
#define s32 int
#define u32 unsigned int
#define v2 vec2
#define v3 vec3
#define v4 vec4
#define V2 vec2
#define V3 vec3
#define V4 vec4
#define Matrix2 mat2
#define Matrix3 mat3
#define Matrix4 mat4
#define v2i ivec2
#define V2i ivec2

#define Lerp(a, t, b) mix(a, b, t)
#define Clamp01(t) clamp(t, 0, 1)
#define Clamp(t, min, max) clamp(t, min, max)
#define Inner(a, b) dot(a, b)
#define Length(a) length(a)
#define LengthSq(a) dot(a, a)
#define Normalize(a) normalize(a)
#define SquareRoot(a) sqrt(a)
#define Maximum(a, b) max(a, b)
#define Minimum(a, b) min(a, b)
#define AbsoluteValue(a) abs(a)
#define Pow(a, b) pow(a, b)

struct light_shader_data
{
	b32 IsDirectional;
	v3 Color;
	f32 Intensity;

	v3 Direction;
	f32 FuzzFactor;
	v3 P;
	f32 Size;
	f32 LinearAttenuation;
	f32 QuadraticAttenuation;
	f32 Radius;
};

struct decal_shader_data
{
	mat4 Projection;
	mat4 View;
};

v3 UnpackNormal(v2 PackedNormal)
{
	// TODO: Needs to deal with the sign of Z!!!
	f32 Z = sqrt(1.0f - PackedNormal.x*PackedNormal.x + PackedNormal.y*PackedNormal.y);
	v3 Result = V3(PackedNormal, Z);
	return Result;
}

#line 1
