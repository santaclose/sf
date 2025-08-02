layout(location = 0) out vec2 fScreenPos;

layout(binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

void main()
{
	gl_Position = cameraMatrix * modelMatrix * vec4(VA_Position, 1.0);
	fScreenPos = gl_Position.xy;
}