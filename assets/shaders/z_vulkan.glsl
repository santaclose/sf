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

layout(binding = 0) uniform SharedGpuData
{
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	mat4 skyboxMatrix;
	vec3 cameraPosition;
} sgd;

layout(std140, set = 1, binding = 0) readonly buffer ParticleMatrices
{
	mat4 particleMatrices[];
} pm;

layout( push_constant ) uniform constants
{
	mat4 modelMatrix;
	int vertexShaderId;
	int fragmentShaderId;
} pc;

#define VA_POSITION vPosition
#define VA_NORMAL vNormal
#define VA_TANGENT vTangent
#define VA_BITANGENT vBitangent
#define VA_COLOR vColor
#define VA_TEX_COORDS vTexCoords
#define VA_AMBIENT_OCCLUSION vAo
#define VA_BONE_INDICES vBoneIndices
#define VA_BONE_WEIGHTS vBoneWeights

#define OBJECT_MATRIX pc.modelMatrix
#define CAMERA_MATRIX sgd.cameraMatrix
#define CAMERA_POSITION sgd.cameraPosition
#define SCREEN_SPACE_MATRIX sgd.screenSpaceMatrix
#define SKYBOX_MATRIX sgd.skyboxMatrix

#define VERTEX_SHADER_ID pc.vertexShaderId
#define FRAGMENT_SHADER_ID pc.fragmentShaderId

#define PARTICLE_MATRICES pm.particleMatrices
#define SKINNING_MATRICES pm.particleMatrices

#define INSTANCE_INDEX gl_InstanceIndex

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define PI 3.14159265359