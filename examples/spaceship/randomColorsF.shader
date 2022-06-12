#version 430 core

out vec4 color;

in vec4 fragColor;

void main()
{
	color = fragColor;
}