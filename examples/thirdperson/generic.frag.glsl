#include <assets/shaders/quaternion.h>

layout(location = 3) in vec3 fWorldPos;
layout(location = 4) in vec2 fTexCoords;
layout(location = 5) in float fVertexAo;

uniform bool useColorSampler = false;
uniform sampler2D colorSampler;
uniform vec3 color = vec3(1.0, 1.0, 1.0);

uniform bool useAoSampler = true;
uniform sampler3D aoSampler;
uniform vec3 aoSamplerOrigin;
uniform vec3 aoSamplerSize;
uniform vec3 aoSamplerModelPosition;
uniform vec4 aoSamplerModelRotation;

void main()
{
	float raw = fVertexAo;
	if (useAoSampler)
	{
		vec4 rotUndo = QuatConjugate(aoSamplerModelRotation);
		vec3 vecModelSpace = QuatRotateVector(rotUndo, fWorldPos - aoSamplerModelPosition);
		vec3 sampleVector = (vecModelSpace - aoSamplerOrigin) / aoSamplerSize;
		raw *= texture(aoSampler, sampleVector).r;
	}

	vec3 finalColor = useColorSampler ? texture(colorSampler, fTexCoords).rgb : color;
	// vec3 finalColor = texture(colorSampler, fTexCoords).rgb;
	float corrected = pow(raw, 1.0 / 2.2);
	// OUT_COLOR = vec4(vec3(pow(raw, 1.0 / 2.2)), 1.0);
	OUT_COLOR = vec4(finalColor * corrected, 1.0);
}