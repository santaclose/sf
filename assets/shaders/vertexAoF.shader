#version 430 core

out vec4 color;

in float vertexAo;

void main()
{
	color = vec4(vec3(pow(vertexAo, 1.0 / 2.2)), 1.0);
}