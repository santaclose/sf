#version 450
#define VERTEX
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

layout(std140, set = 0, binding = 1) readonly buffer ParticleMatrices
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
// layout(set = 0, binding = 2) uniform UniformBlock {
// 	bool animate; //= false;
// };
layout(location = 0) out VertexOut {
	float fVertexAofloat;
	vec2 fTexCoordsvec2;
	vec3 fWorldPosvec3;
	mat3 fTBNmat3;
	vec3 fTexCoordsvec3;
};




void main0()
{
	fTexCoordsvec2 = VA_TEX_COORDS;
	fVertexAofloat = VA_AMBIENT_OCCLUSION;

	mat4 skinMat = 1 ?
		VA_BONE_WEIGHTS.x * SKINNING_MATRICES[int(VA_BONE_INDICES.x)] +
		VA_BONE_WEIGHTS.y * SKINNING_MATRICES[int(VA_BONE_INDICES.y)] +
		VA_BONE_WEIGHTS.z * SKINNING_MATRICES[int(VA_BONE_INDICES.z)] +
		VA_BONE_WEIGHTS.w * SKINNING_MATRICES[int(VA_BONE_INDICES.w)] : mat4(1.0);

	vec3 T = normalize(vec3(OBJECT_MATRIX * skinMat * vec4(VA_TANGENT, 0.0)));
	vec3 B = normalize(vec3(OBJECT_MATRIX * skinMat * vec4(VA_BITANGENT, 0.0)));
	vec3 N = normalize(vec3(OBJECT_MATRIX * skinMat * vec4(VA_NORMAL, 0.0)));
	fTBNmat3 = mat3(T, B, N);

	// standard normal passing
	//normal = (OBJECT_MATRIX * vec4(VA_NORMAL, 0.0)).xyz;
	// for allowing non uniform scaling
	//normal = (transpose(inverse(OBJECT_MATRIX))* VA_NORMAL).xyz;

	fWorldPosvec3 = (OBJECT_MATRIX * skinMat * vec4(VA_POSITION, 1.0)).rgb;
	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * skinMat * vec4(VA_POSITION, 1.0);
}


void main1()
{
	fVertexAofloat = VA_AMBIENT_OCCLUSION;
	fTexCoordsvec2 = VA_TEX_COORDS;
	fWorldPosvec3 = (OBJECT_MATRIX * vec4(VA_POSITION, 1.0)).rgb;

	vec3 T = normalize(vec3(OBJECT_MATRIX * vec4(VA_TANGENT, 0.0)));
	vec3 B = normalize(vec3(OBJECT_MATRIX * vec4(VA_BITANGENT, 0.0)));
	vec3 N = normalize(vec3(OBJECT_MATRIX * vec4(VA_NORMAL, 0.0)));
	fTBNmat3 = mat3(T, B, N);

	// standard normal passing
	//normal = (OBJECT_MATRIX * vec4(VA_NORMAL, 0.0)).xyz;
	// for allowing non uniform scaling
	//normal = (transpose(inverse(OBJECT_MATRIX))* VA_NORMAL).xyz;

	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * vec4(VA_POSITION, 1.0);
}


void main2()
{
	fTexCoordsvec3 = VA_POSITION;
	gl_Position = SKYBOX_MATRIX * vec4(VA_POSITION, 1.0);
}


void main3()
{
	fTexCoordsvec2 = VA_TEX_COORDS;
	gl_Position = SCREEN_SPACE_MATRIX * vec4(VA_POSITION.xy, 0.0, 1.0);
}


void main4()
{
	fTexCoordsvec2 = VA_TEX_COORDS;
	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * PARTICLE_MATRICES[INSTANCE_INDEX] * vec4(VA_POSITION, 1.0);
}
void main()
{
#ifdef VERTEX
	switch (VERTEX_SHADER_ID)
#else
	switch (FRAGMENT_SHADER_ID)
#endif
	{
		case 0: main0(); break;
		case 1: main1(); break;
		case 2: main2(); break;
		case 3: main3(); break;
		case 4: main4(); break;
	}
}
// __sh__ assets/shaders/defaultSkinningV.glsl
// __sh__ assets/shaders/defaultV.glsl
// __sh__ assets/shaders/skyboxV.glsl
// __sh__ assets/shaders/spriteV.glsl
// __sh__ assets/shaders/voxelBoxV.glsl