#version 460
#include "../../assets/shaders/shaderCommon.h"

layout(location = 0) out vec4 fColor;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec3 vColor;
layout(location = 5) in vec2 vTexCoords;
layout(location = 6) in float vAmbientOcclusion;

float random(vec2 st) {
	return fract(sin(dot(st.xy,
		vec2(12.9898, 78.233))) *
		43758.5453123);
}

void main()
{
	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * vec4(vPosition, 1.0);
	fColor = vec4(random(vec2(vPosition.x, vPosition.y)), random(vec2(vPosition.y, vPosition.x)), random(vec2(vPosition.x * 2.0, vPosition.y)), 1.0);
}