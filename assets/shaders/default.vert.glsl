#include <assets/shaders/transform.h>

layout(location = 0) out mat3 fTBN;
layout(location = 3) out vec3 fWorldPos;
layout(location = 4) out vec2 fTexCoords;
layout(location = 5) out float fVertexAo;

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

layout (binding = 1) buffer SkinningTransformsBuffer
{
	float skinningTransforms[];
};

#if HAS_VA_BoneIndices && HAS_VA_BoneWeights
uniform bool animate = false;
#endif

void main()
{
	Transform modelTr;
	TRANSFORM_LOAD_FROM_ARRAY(modelTr, 0, modelTransform);
	mat4 modelMatrix = TransformToMatrix(modelTr);
#if HAS_VA_UV
	fTexCoords = VA_UV;
#endif

#if HAS_VA_AO
	fVertexAo = VA_AO;
#endif

#if HAS_VA_BoneIndices && HAS_VA_BoneWeights
	Transform transformArray[4];
	TRANSFORM_LOAD_FROM_ARRAY(transformArray[0], int(VA_BoneIndices.x), skinningTransforms);
	TRANSFORM_LOAD_FROM_ARRAY(transformArray[1], int(VA_BoneIndices.y), skinningTransforms);
	TRANSFORM_LOAD_FROM_ARRAY(transformArray[2], int(VA_BoneIndices.z), skinningTransforms);
	TRANSFORM_LOAD_FROM_ARRAY(transformArray[3], int(VA_BoneIndices.w), skinningTransforms);
	// mat4 skinMat = animate ?
	// 	VA_BoneWeights.x * TransformToMatrix(transformArray[0]) +
	// 	VA_BoneWeights.y * TransformToMatrix(transformArray[1]) +
	// 	VA_BoneWeights.z * TransformToMatrix(transformArray[2]) +
	// 	VA_BoneWeights.w * TransformToMatrix(transformArray[3]) : mat4(1.0);

	Transform skinTr = animate ? TransformWeightedBlend(transformArray, VA_BoneWeights) : TRANSFORM_IDENTITY;
	skinTr.scale = 1.0f;
#else
	// mat4 skinMat = mat4(1.0);
	Transform skinTr = TRANSFORM_IDENTITY;
#endif

	vec4 finalNormalRotation = QuatMultiply(modelTr.rotation, skinTr.rotation);
#if HAS_VA_Normal
	vec3 N = QuatRotateVector(finalNormalRotation, VA_Normal);
	// vec3 N = normalize(vec3(modelMatrix * skinMat * vec4(VA_Normal, 0.0)));
#if HAS_VA_Tangent
	vec3 T = QuatRotateVector(finalNormalRotation, VA_Tangent);
	vec3 B = QuatRotateVector(finalNormalRotation, cross(VA_Normal, VA_Tangent));
	// vec3 T = normalize(vec3(modelMatrix * skinMat * vec4(VA_Tangent, 0.0)));
	// vec3 B = normalize(vec3(modelMatrix * skinMat * vec4(cross(VA_Normal, VA_Tangent), 0.0)));
#else
	vec3 helper = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 T = normalize(cross(helper, N));
	vec3 B = cross(N, T);
#endif
	fTBN = mat3(T, B, N);
#endif
	// skinTr.rotation = QUATERNION_IDENTITY;
	skinTr.scale = 1.0f;
	skinTr.position = vec3(0.0);
	// skinTr.rotation = QUATERNION_IDENTITY;
	skinTr.rotation = normalize(skinTr.rotation);
	mat4 skinMat = TransformToMatrix(skinTr);
	// skinMat = mat4(1.0);


	Transform vertexTr = TRANSFORM_IDENTITY;
	vertexTr.position = VA_Position;
	vertexTr = TransformApply(skinTr, vertexTr);
	vertexTr = TransformApply(modelTr, vertexTr);

	fWorldPos = vertexTr.position;
	gl_Position = cameraMatrix * vec4(vertexTr.position, 1.0);

	// fWorldPos = (modelMatrix * skinMat * vec4(VA_Position, 1.0)).rgb;
	// gl_Position = cameraMatrix * modelMatrix * skinMat * vec4(VA_Position, 1.0);
}