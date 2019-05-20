#ifdef VERTEX_SHADER

layout (location = 0) in v3 Position;
layout (location = 1) in v3 Normal;
layout (location = 2) in v2 TexCoords;
layout (location = 3) in v4 Color;

out v2 TexCoordsToFragment;

uniform mat4 Transform;

void main()
{
    gl_Position = Transform * V4(Position, 1.0f);
	TexCoordsToFragment = TexCoords;
}


#endif
#ifdef FRAGMENT_SHADER

layout(location = 0) out v4 FragColor;

in v2 TexCoordsToFragment;

uniform sampler2DMS ColorTexture;
uniform sampler2DMS DepthTexture;

#define DEPTH_THRESHOLD 0.001f

void main()
{
	FragColor = V4(0,0,0,0);
	v2i TexelCoords = V2i(gl_FragCoord.xy);

	f32 DepthMin = 1.0f;
	f32 DepthMax = 0.0f;
	for(s32 SampleIndex = 0;
		SampleIndex < MULTISAMPLE_COUNT;
		++SampleIndex)
	{
		f32 Depth = texelFetch(DepthTexture, TexelCoords, SampleIndex).r;
		DepthMin = Minimum(DepthMin, Depth);
		DepthMax = Maximum(DepthMax, Depth);
	}

	s32 Count = 0;
	for(s32 SampleIndex = 0;
		SampleIndex < MULTISAMPLE_COUNT;
		++SampleIndex)
	{
		f32 Depth = texelFetch(DepthTexture, TexelCoords, SampleIndex).r;
		if(Depth <= (DepthMin + DEPTH_THRESHOLD))
		{
			FragColor += texelFetch(ColorTexture, TexelCoords, SampleIndex);
			++Count;
		}
	}

	FragColor /= MULTISAMPLE_COUNT;
	gl_FragDepth = 0.5f*(DepthMin + DepthMax);
	gl_FragDepth = DepthMin;
}

#endif
