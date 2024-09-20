#version 450

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec3 vColor;
layout(location = 5) in vec2 vTexCoords;
layout(location = 6) in float vAmbientOcclusion;

layout(binding = 0) uniform SharedGpuData
{
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
} sgd;

layout(push_constant) uniform constants
{
	mat4 modelMatrix;
} pc;


layout(location = 0) out mat3 fTBN;
layout(location = 3) out vec3 fWorldPos;
layout(location = 4) out vec2 fTexCoords;
layout(location = 5) out float fVertexAo;

void main()
{
	fVertexAo = vAmbientOcclusion;
	fTexCoords = vTexCoords;
	fWorldPos = (pc.modelMatrix * vec4(vPosition, 1.0)).rgb;

	vec3 T = normalize(vec3(pc.modelMatrix * vec4(vTangent, 0.0)));
	vec3 B = normalize(vec3(pc.modelMatrix * vec4(vBitangent, 0.0)));
	vec3 N = normalize(vec3(pc.modelMatrix * vec4(vNormal, 0.0)));
	fTBN = mat3(T, B, N);

	// standard normal passing
	//normal = (pc.modelMatrix * vec4(VA_NORMAL, 0.0)).xyz;
	// for allowing non uniform scaling
	//normal = (transpose(inverse(modelMatrix))* VA_NORMAL).xyz;

	gl_Position = sgd.cameraMatrix * pc.modelMatrix * vec4(vPosition, 1.0);
}