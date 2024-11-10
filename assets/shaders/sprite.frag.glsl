#version 460

layout(location = 0) out vec4 outColor;

layout(location = 4) in vec2 fTexCoords;

uniform sampler2D bitmap;

void main()
{
	outColor = texture(bitmap, fTexCoords);
}