#version 430 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec3 vColor;
layout(location = 5) in vec2 vTexCoords;
layout(location = 6) in float vAo;

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

out mat3 fTBN;
out vec3 fWorldPos;
out vec2 fTexCoords;
out float fVertexAo;

void main()
{
	fVertexAo = vAo;
	fTexCoords = vTexCoords;
	fWorldPos = (modelMatrix * vec4(vPosition, 1.0)).rgb;

	vec3 T = normalize(vec3(modelMatrix * vec4(vTangent, 0.0)));
	vec3 B = normalize(vec3(modelMatrix * vec4(vBitangent, 0.0)));
	vec3 N = normalize(vec3(modelMatrix * vec4(vNormal, 0.0)));
	fTBN = mat3(T, B, N);

	// standard normal passing
	//normal = (modelMatrix * vec4(vNormal, 0.0)).xyz;
	// for allowing non uniform scaling
	//normal = (transpose(inverse(modelMatrix))* vNormal).xyz;

	gl_Position = cameraMatrix * modelMatrix * vec4(vPosition, 1.0);
}