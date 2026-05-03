layout(binding = 0) uniform SharedGpuData
{
	mat4 cameraMatrix;
	float modelTransform[8];
	float cameraPositionX;
	float cameraPositionY;
	float cameraPositionZ;
	float windowSizeX;
	float windowSizeY;
};

uniform sampler2D heightmapTexture;
uniform float maxHeight;

in TCS_OUT {
	vec3 pos;
	vec2 uv;
} tese_in[];

out TE_OUT {
	vec3 worldPos;
	vec2 uv;
} te_out;

float sampleHeight(vec2 uv)
{
	return texture(heightmapTexture, uv).r * maxHeight;
}

void main()
{
	vec3 p;
	vec2 uv;

	/* Interpolate attributes */
	if (VERTICES_PER_PATCH == 3)
	{
		p = tese_in[0].pos * gl_TessCoord[0] +
			tese_in[1].pos * gl_TessCoord[1] +
			tese_in[2].pos * gl_TessCoord[2];
		uv = tese_in[0].uv * gl_TessCoord[0] +
			tese_in[1].uv * gl_TessCoord[1] +
			tese_in[2].uv * gl_TessCoord[2];
	}
	else
	{
		/* VERTICES_PER_PATCH should be 4 */
		p = mix(mix(tese_in[0].pos, tese_in[1].pos, gl_TessCoord[0]),
				mix(tese_in[3].pos, tese_in[2].pos, gl_TessCoord[0]),
				gl_TessCoord[1]);
		uv = mix(mix(tese_in[0].uv, tese_in[1].uv, gl_TessCoord[0]),
				mix(tese_in[3].uv, tese_in[2].uv, gl_TessCoord[0]),
				gl_TessCoord[1]);
	}

	float h = sampleHeight(uv);
	vec3 worldPos = p + vec3(0.0, 1.0, 0.0) * h;

	te_out.worldPos = worldPos;
	te_out.uv = uv;

	gl_Position = cameraMatrix * vec4(worldPos, 1.0);
}