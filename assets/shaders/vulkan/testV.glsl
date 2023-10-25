#version 450

layout(binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
} sgd;

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec3 vColor;

layout(location = 0) out vec3 fragColor;

void main() {
	fragColor = vColor;
// 	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
	gl_Position = sgd.cameraMatrix * sgd.modelMatrix * vec4(vPosition,0.0, 1.0);
}