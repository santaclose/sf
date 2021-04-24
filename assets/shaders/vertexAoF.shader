#version 450 core

out vec4 color;

in vec2 texCoord;
in mat3 TBN;
in vec2 extraData;

void main()
{
	//color = vec4(vec3(extraData.x), 1.0);
	color = vec4(vec3(pow(extraData.x, 1.0 / 2.2)), 1.0);
}