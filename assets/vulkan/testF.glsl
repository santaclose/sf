#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UserGpuData
{
	float time;
} ugd;

void main()
{
	outColor = vec4(vec3(
		fragColor.x * sin(ugd.time) / 0.5 + 0.5,
		fragColor.y * sin(ugd.time) / 0.5 + 0.5,
		fragColor.z * sin(ugd.time) / 0.5 + 0.5),
		1.0);
}