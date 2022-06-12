#version 430 core

in vec2 screenPos;
out vec4 color;

uniform vec3 camPos;
uniform vec4 theColor;

void main()
{
	color = theColor;
}