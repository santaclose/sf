#version 430 core

uniform sampler2D albedo;
uniform sampler2D normalMap;

//in vec3 normal;
in vec2 texCoord;
in mat3 TBN;
out vec4 color;

void main()
{
	color = vec4(texCoord.x, texCoord.y, 0.0, 1.0);
}