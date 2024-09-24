#version 460

layout(location = 0) out vec4 outColor;

layout(location = 5) in float fVertexAo;

void main()
{
	outColor = vec4(vec3(pow(fVertexAo, 1.0 / 2.2)), 1.0);
}