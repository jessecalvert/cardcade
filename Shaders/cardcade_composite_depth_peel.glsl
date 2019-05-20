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

layout(location = 0) out v3 FragColor;

in v2 TexCoordsToFragment;

uniform s32 MinPeel;
uniform sampler2D DepthPeels[DEPTH_PEEL_COUNT];

void main()
{
	FragColor = V3(0,0,0);
	v2i TexelCoords = V2i(gl_FragCoord.xy);

	for(s32 PeelIndex = DEPTH_PEEL_COUNT - 1;
		PeelIndex >= MinPeel;
		--PeelIndex)
	{
		v4 Color = texelFetch(DepthPeels[PeelIndex], TexelCoords, 0);
		FragColor = Lerp(FragColor, Color.a, Color.rgb);
	}
}

#endif
