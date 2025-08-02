layout(location = 0) out vec4 fColor;

layout(binding = 0) uniform SharedGpuData
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
	gl_Position = cameraMatrix * modelMatrix * vec4(VA_Position, 1.0f);
	fColor = vec4(random(vec2(VA_Position.x, VA_Position.y)), random(vec2(VA_Position.y, VA_Position.x)), random(vec2(VA_Position.x * 2.0, VA_Position.y)), 1.0);
}