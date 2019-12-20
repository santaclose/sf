#version 330 core

out vec4 color;

in vec4 fragColor;

uniform vec3 camPos;

void main()
{
	//color = vec4(1.0, 0.0, 1.0, 1.0);
	color = fragColor;
}