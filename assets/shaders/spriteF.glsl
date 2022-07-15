#version 430 core

in vec2 fTextureCoords;
out vec4 outColor;

uniform sampler2D bitmap;

void main()
{
	outColor = texture(bitmap, fTextureCoords);
}