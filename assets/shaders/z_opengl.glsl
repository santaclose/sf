#ifdef VERTEX
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec3 vColor;
layout(location = 5) in vec2 vTexCoords;
layout(location = 6) in float vAo;
layout(location = 7) in vec4 vBoneIndices;
layout(location = 8) in vec4 vBoneWeights;
#endif

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

layout (std430, binding = 1) buffer ParticleMatricesBuffer
{
	mat4 particleMatrices[];
};

#define VA_POSITION vPosition
#define VA_NORMAL vNormal
#define VA_TANGENT vTangent
#define VA_BITANGENT vBitangent
#define VA_COLOR vColor
#define VA_TEX_COORDS vTexCoords
#define VA_AMBIENT_OCCLUSION vAo
#define VA_BONE_INDICES vBoneIndices
#define VA_BONE_WEIGHTS vBoneWeights

#define OBJECT_MATRIX modelMatrix
#define CAMERA_MATRIX cameraMatrix
#define CAMERA_POSITION cameraPosition
#define SCREEN_SPACE_MATRIX screenSpaceMatrix
uniform mat4 SKYBOX_MATRIX;

#define PARTICLE_MATRICES particleMatrices
#define SKINNING_MATRICES particleMatrices