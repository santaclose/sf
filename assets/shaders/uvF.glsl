#version 430 core

in vec2 fTextureCoords;
out vec4 outColor;

void main()
{
	outColor = vec4(fTextureCoords.x, fTextureCoords.y, 0.0, 1.0);
}