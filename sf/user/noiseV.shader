#version 330 core

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec4 aNormal;
layout(location = 3) in vec2 aTextureCoord;


/*out vec3 position;
out vec3 normal;
out vec2 textureCoord;*/

out vec2 screenPos;

uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

void main()
{
	/*position = vec3(aPosition);
	normal = vec3(aNormal);
	textureCoord = aTextureCoord;*/

	gl_Position = cameraMatrix * modelMatrix * aPosition;
	screenPos = gl_Position.xy;
}