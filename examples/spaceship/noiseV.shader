#version 430 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBitangent;
layout(location = 4) in vec2 aTextureCoord;
layout(location = 5) in vec2 aExtraData;

out vec2 screenPos;

layout(std430, binding = 0) buffer SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	vec3 cameraPosition;
};

void main()
{
	gl_Position = cameraMatrix * modelMatrix * vec4(aPosition, 1.0);
	screenPos = gl_Position.xy;
}