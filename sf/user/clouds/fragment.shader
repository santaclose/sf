#version 430 core

out vec4 color;

uniform vec4 icolor;

void main()
{
	color = icolor;
}