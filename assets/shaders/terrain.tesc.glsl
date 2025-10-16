in VS_OUT {
	vec3 pos;
	vec2 uv;
} tcs_in[];

out TCS_OUT {
	vec3 pos;
	vec2 uv;
} tcs_out[];

layout(binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	float cameraPositionX;
	float cameraPositionY;
	float cameraPositionZ;
	float windowSizeX;
	float windowSizeY;
};

uniform float maxResDistance = 70.0;
uniform float minResDistance = 100.0;
uniform float minRes = 10.0;
uniform float maxRes = 64.0;

float computeTessLevel(vec3 edgeCenter)
{
	float d = length(vec3(cameraPositionX, 0.0, cameraPositionZ) - vec3(edgeCenter.x, 0.0, edgeCenter.z));
	d = clamp(d, maxResDistance, minResDistance);
	float alpha = (d - maxResDistance) / (minResDistance - maxResDistance);
	alpha = 1.0 - alpha;
	return alpha * (maxRes - minRes) + minRes;
}

void main()
{
	// pass through per-vertex data
	tcs_out[gl_InvocationID].pos = tcs_in[gl_InvocationID].pos;
	tcs_out[gl_InvocationID].uv  = tcs_in[gl_InvocationID].uv;

	if (gl_InvocationID == 0)
	{
		if (VERTICES_PER_PATCH == 3)
		{
			vec3 edgeMidA = (tcs_in[0].pos + tcs_in[1].pos) * 0.5;
			vec3 edgeMidB = (tcs_in[1].pos + tcs_in[2].pos) * 0.5;
			vec3 edgeMidC = (tcs_in[2].pos + tcs_in[0].pos) * 0.5;

			gl_TessLevelOuter[0] = computeTessLevel(edgeMidB);
			gl_TessLevelOuter[1] = computeTessLevel(edgeMidC);
			gl_TessLevelOuter[2] = computeTessLevel(edgeMidA);
			gl_TessLevelInner[0] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[1] + gl_TessLevelOuter[2]) / 3.0;
		}
		else
		{
			/* VERTICES_PER_PATCH should be 4 */
			vec3 edgeMidA = (tcs_in[0].pos + tcs_in[1].pos) * 0.5;
			vec3 edgeMidB = (tcs_in[1].pos + tcs_in[2].pos) * 0.5;
			vec3 edgeMidC = (tcs_in[2].pos + tcs_in[3].pos) * 0.5;
			vec3 edgeMidD = (tcs_in[3].pos + tcs_in[0].pos) * 0.5;

			gl_TessLevelOuter[0] = computeTessLevel(edgeMidD);
			gl_TessLevelOuter[1] = computeTessLevel(edgeMidA);
			gl_TessLevelOuter[2] = computeTessLevel(edgeMidB);
			gl_TessLevelOuter[3] = computeTessLevel(edgeMidC);
			gl_TessLevelInner[0] = gl_TessLevelInner[1] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[1] + gl_TessLevelOuter[2] + gl_TessLevelOuter[3]) / 4.0;
		}
	}
}