#version 430 core

out vec4 outColor;
in vec3 fTexCoords;

uniform samplerCube skybox;

void main()
{
	outColor = texture(skybox, fTexCoords);
}