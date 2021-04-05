#version 330 core

out vec4 color;
in vec3 worldPos;
in vec2 texCoord;
in mat3 TBN;

uniform vec3 u_Color = vec3(0.0, 0.0, 0.0);

void main()
{
	color = vec4(u_Color, 1.0);
}