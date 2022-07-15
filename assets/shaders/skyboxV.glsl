#version 430 core

layout(location = 0) in vec3 vPosition;

out vec3 fTextureCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	fTextureCoords = vPosition;
	gl_Position = projection * view * vec4(vPosition, 1.0);
}