#version 430 core

in vec2 texCoord;
out vec4 color;

uniform sampler2D bitmap;

void main()
{
	color = texture(bitmap, texCoord);
}