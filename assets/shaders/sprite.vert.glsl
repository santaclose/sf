#version 460
#include "../../assets/shaders/shaderCommon.h"

layout(location = 4) out vec2 fTexCoords;

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vTexCoords;

void main()
{
	fTexCoords = vTexCoords;
	gl_Position = SCREEN_SPACE_MATRIX * vec4(vPosition, 0.0, 1.0);
}