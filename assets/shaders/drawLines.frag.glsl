#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fColor;

void main()
{
	outColor = vec4(fColor, 1.0);
}