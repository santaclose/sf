#version 430 core

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec4 aNormal;
layout(location = 2) in vec2 aTextureCoord;

out vec3 position;
out vec3 normal;
out vec4 fragColor;
out vec2 textureCoord;

layout(std430, binding = 0) buffer SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	vec3 cameraPosition;
};

float random(vec2 st) {
	return fract(sin(dot(st.xy,
		vec2(12.9898, 78.233))) *
		43758.5453123);
}

void main()
{
	position = vec3(aPosition);
	normal = vec3(aNormal);
	textureCoord = aTextureCoord;

	gl_Position = cameraMatrix * modelMatrix * aPosition;
	fragColor = vec4(random(vec2(position.x, position.y)), random(vec2(position.y, position.x)), random(vec2(position.x * 2.0, position.y)), 1.0);
}