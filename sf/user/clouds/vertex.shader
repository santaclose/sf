#version 430 core

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBitangent;
layout(location = 4) in vec2 aTextureCoord;

uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

void main()
{
	gl_Position = cameraMatrix * modelMatrix * aPosition;
}