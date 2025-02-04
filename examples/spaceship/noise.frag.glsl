#version 460

#include <assets/shaders/random.h>

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fScreenPos;

void main()
{
	outColor = vec4(vec3(Random(fScreenPos)), 1.0);
}