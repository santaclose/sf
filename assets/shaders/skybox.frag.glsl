#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fTexCoords;

uniform samplerCube skybox;

void main()
{
	outColor = texture(skybox, fTexCoords);
}