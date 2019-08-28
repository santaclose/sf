#version 330 core

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec4 aNormal;
layout(location = 2) in vec2 aTextureCoord;

out vec3 normal;
out vec2 texCoord;

uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

void main()
{
	texCoord = aTextureCoord;

	normal = (modelMatrix * vec4(aNormal.xyz, 0.0)).xyz;
	//normal = (transpose(inverse(modelMatrix))* aNormal).xyz;

	gl_Position = cameraMatrix * modelMatrix * aPosition;
}