#version 460

layout(location = 0) out vec3 fTexCoords;

layout(location = 0) in vec3 vPosition;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	fTexCoords = vPosition;
	gl_Position = projection * view * vec4(vPosition, 1.0);
}