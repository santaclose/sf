#include <assets/shaders/quaternion.h>

layout(location = 3) in vec3 fWorldPos;
layout(location = 5) in float fVertexAo;

uniform bool useSampler = true;
uniform sampler3D aoSampler;
uniform vec3 aoSamplerOrigin;
uniform vec3 aoSamplerSize;
uniform vec3 aoSamplerModelPosition;
uniform vec4 aoSamplerModelRotation;

void main()
{
	float raw = fVertexAo;
	if (useSampler)
	{
		vec4 rotUndo = QuatConjugate(aoSamplerModelRotation);
		vec3 vecModelSpace = QuatRotateVector(rotUndo, fWorldPos - aoSamplerModelPosition);
		vec3 sampleVector = (vecModelSpace - aoSamplerOrigin) / aoSamplerSize;
		raw *= texture(aoSampler, sampleVector).r;
	}

	OUT_COLOR = vec4(vec3(pow(raw, 1.0 / 2.2)), 1.0);
}