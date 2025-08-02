layout(location = 0) out mat3 fTBN;
layout(location = 3) out vec3 fWorldPos;
layout(location = 4) out vec2 fTexCoords;
layout(location = 5) out float fVertexAo;

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

layout (binding = 1) buffer SkinningMatricesBuffer
{
	mat4 skinningMatrices[];
};

#if HAS_VA_BoneIndices && HAS_VA_BoneWeights
uniform bool animate = false;
#endif

void main()
{
#if HAS_VA_UV
	fTexCoords = VA_UV;
#endif

#if HAS_VA_AO
	fVertexAo = VA_AO;
#endif

#if HAS_VA_BoneIndices && HAS_VA_BoneWeights
	mat4 skinMat = animate ?
		VA_BoneWeights.x * skinningMatrices[int(VA_BoneIndices.x)] +
		VA_BoneWeights.y * skinningMatrices[int(VA_BoneIndices.y)] +
		VA_BoneWeights.z * skinningMatrices[int(VA_BoneIndices.z)] +
		VA_BoneWeights.w * skinningMatrices[int(VA_BoneIndices.w)] : mat4(1.0);
#else
	mat4 skinMat = mat4(1.0);
#endif

#if HAS_VA_Normal
	vec3 N = normalize(vec3(modelMatrix * skinMat * vec4(VA_Normal, 0.0)));
#if HAS_VA_Tangent
	vec3 T = normalize(vec3(modelMatrix * skinMat * vec4(VA_Tangent, 0.0)));
	vec3 B = normalize(vec3(modelMatrix * skinMat * vec4(cross(VA_Normal, VA_Tangent), 0.0)));
#else
	vec3 helper = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 T = normalize(cross(helper, N));
	vec3 B = cross(N, T);
#endif
	fTBN = mat3(T, B, N);
#endif

	fWorldPos = (modelMatrix * skinMat * vec4(VA_Position, 1.0)).rgb;
	gl_Position = cameraMatrix * modelMatrix * skinMat * vec4(VA_Position, 1.0);
}