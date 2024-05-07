#version 430 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoords;

out vec2 fragInPos;
flat out int fragInDataOffset;

struct InstanceData
{
    vec2 boundsSize;
    vec2 offset;
    float lineAdvance;
    float letterAdvance;
    float wordAdvance;
    int contourDataOffset;
};

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};
layout (std430, binding = 1) buffer instanceSSBO
{
	InstanceData PerInstanceData[];
};

layout(std140, binding = 4) uniform TextLayoutData
{
    float fontSize;
    float lineSpacing;
    float letterSpacing;
    float wordSpacing;
};

uniform vec2 globalOffset = vec2(0.0, 0.0);


float GetAdvanceX(float letterAdvance, float wordAdvance, float offsetX)
{
    return (letterAdvance * letterSpacing + wordAdvance * wordSpacing + offsetX) * fontSize;
}

float GetAdvanceY(float lineAdvance, float offsetY)
{
    return (-lineAdvance * lineSpacing + offsetY) * fontSize;
}

void main()
{
	InstanceData instanceData = PerInstanceData[gl_InstanceID];

	vec2 instancePos = vec2(
		GetAdvanceX(instanceData.letterAdvance, instanceData.wordAdvance, instanceData.offset.x),
		GetAdvanceY(instanceData.lineAdvance, instanceData.offset.y)
	);

	vec2 offset = instancePos + vec2(globalOffset.x, -globalOffset.y);
	vec2 instanceVertPos = vPosition.xy * instanceData.boundsSize * fontSize + offset;
	instanceVertPos.y = -instanceVertPos.y;
	instanceVertPos = (screenSpaceMatrix * vec4(instanceVertPos.xy, 0.0, 1.0)).xy;

	gl_Position = vec4(instanceVertPos.x, instanceVertPos.y, 0.0, 1.0);
	fragInPos = -instanceData.boundsSize / 2 + instanceData.boundsSize * vTexCoords;
	fragInDataOffset = instanceData.contourDataOffset;
}