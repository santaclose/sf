#version 460

layout(location = 0) out vec4 outColor;
layout(location = 4) in vec2 fTexCoords;

void main()
{
	outColor = vec4(fTexCoords.x, fTexCoords.y, 0.0, 1.0);
	//outColor = vec4(1.0, 0.0, 1.0, 1.0);
}