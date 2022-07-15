#version 430 core

layout(location = 0) in vec4 vPosition;

out vec4 fColor;

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

float random(vec2 st) {
	return fract(sin(dot(st.xy,
		vec2(12.9898, 78.233))) *
		43758.5453123);
}

void main()
{
	gl_Position = cameraMatrix * modelMatrix * vPosition;
	fColor = vec4(random(vec2(vPosition.x, vPosition.y)), random(vec2(vPosition.y, vPosition.x)), random(vec2(vPosition.x * 2.0, vPosition.y)), 1.0);
}