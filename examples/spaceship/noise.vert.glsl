#version 460
#include "../../assets/shaders/shaderCommon.h"

layout(location = 0) out vec2 fScreenPos;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec3 vColor;
layout(location = 5) in vec2 vTexCoords;
layout(location = 6) in float vAmbientOcclusion;

void main()
{
	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * vec4(vPosition, 1.0);
	fScreenPos = gl_Position.xy;
}