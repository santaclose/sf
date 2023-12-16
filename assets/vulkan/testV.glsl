#version 450


// 		glm::vec3 pos;
// 		glm::vec3 normal;
// 		glm::vec3 tan;
// 		glm::vec3 bitan;
// 		glm::vec3 color;
// 		glm::vec2 uv;
// 		float ao;

layout(binding = 0) uniform SharedGpuData
{
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
} sgd;

layout( push_constant ) uniform constants
{
	mat4 modelMatrix;
	int vertexAttributeArrayOffset;
	int vertexAttributeArrayStride;
} pc;

layout(std140, binding = 2) readonly buffer ParticleMatrices
{
	mat4 particleMatrices[];
} pm;

layout(std140, binding = 3) readonly buffer VertexAttributes
{
	float va[];
} va;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;

layout(location = 0) out vec3 fragColor;

void main() {
	fragColor = vColor;
	
	vec3 vertexPos = vec3(
		va.va[pc.vertexAttributeArrayOffset + pc.vertexAttributeArrayStride * gl_VertexIndex + 0],
		va.va[pc.vertexAttributeArrayOffset + pc.vertexAttributeArrayStride * gl_VertexIndex + 1],
		va.va[pc.vertexAttributeArrayOffset + pc.vertexAttributeArrayStride * gl_VertexIndex + 2]);
	
// 	vec3 vertexPos = vPosition;
	
	gl_Position = sgd.cameraMatrix * pm.particleMatrices[gl_InstanceIndex] * pc.modelMatrix * vec4(vertexPos, 1.0);
}