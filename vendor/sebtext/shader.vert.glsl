#version 460

#include <assets/shaders/shared.h>

layout(location = 0) out vec2 fragInPos;
layout(location = 1) flat out int fragInDataOffset;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoords;

struct InstanceData
{
    vec2 boundsSize;
    vec2 offset;
    float letterAdvance;
    float wordAdvance;
    int contourDataOffset;
	int line;
};

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	float cameraPositionX;
	float cameraPositionY;
	float cameraPositionZ;
	float windowSizeX;
	float windowSizeY;
};

layout (std430, binding = 1) buffer instanceSSBO
{
	InstanceData PerInstanceData[];
};
layout (std430, binding = 4) buffer lastCharPerLineSSBO
{
	int LastCharPerLine[];
};

layout(std140, binding = 5) uniform TextLayoutData
{
    float fontSize;
    float lineSpacing;
    float letterSpacing;
    float wordSpacing;
	int alignmentH; // 0 for left, 1 for middle, 2 for right
	int alignmentV; // 0 for top, 1 for middle, 2 for bottom
};

uniform int lineCount;
uniform vec2 globalOffset;

// LINE_HEIGHT_EM should be consistent with SPACE_SIZE_EM in TextData.cpp
#define LINE_HEIGHT_EM 1.3

float GetAdvanceX(float letterAdvance, float wordAdvance, float offsetX)
{
    return (letterAdvance * letterSpacing + wordAdvance * wordSpacing + offsetX) * fontSize;
}

// Vertical Centered
// lineCount = 1 -> -0.5
// lineCount = 2 -> 0
// lineCount = 3 -> +0.5
// lineCount = 4 -> +1.0

// Vertical Bottom aligned
// lineCount = 1 -> 0
// lineCount = 2 -> +1.0

float GetAdvanceY(int line, float offsetY)
{
	float lineAdvance = float(line) * LINE_HEIGHT_EM;
	float lineAligned = (
		float(alignmentV == 0) * LINE_HEIGHT_EM * -1.0 +
		float(alignmentV == 1) * LINE_HEIGHT_EM * (float(lineCount-2) * 0.5) +
		float(alignmentV == 2) * LINE_HEIGHT_EM * (float(lineCount-1))
		);
    return (lineAligned -lineAdvance * lineSpacing + offsetY) * fontSize;
}

void main()
{
	InstanceData instanceData = PerInstanceData[gl_InstanceID];

	int lastCharForCurrentLine = LastCharPerLine[instanceData.line];
	float textWidth = GetAdvanceX(PerInstanceData[lastCharForCurrentLine].letterAdvance, PerInstanceData[lastCharForCurrentLine].wordAdvance, PerInstanceData[lastCharForCurrentLine].offset.x) + PerInstanceData[lastCharForCurrentLine].boundsSize.x * fontSize;

	vec2 instancePos = vec2(
		GetAdvanceX(instanceData.letterAdvance, instanceData.wordAdvance, instanceData.offset.x),
		GetAdvanceY(instanceData.line, instanceData.offset.y)
	);

	vec2 offset = instancePos + vec2(globalOffset.x, -globalOffset.y);
	vec2 instanceVertPos = vPosition.xy * instanceData.boundsSize * fontSize + offset - vec2(float(alignmentH == 2) * textWidth + float(alignmentH == 1) * textWidth / 2.0, 0.0);
	instanceVertPos.y = -instanceVertPos.y;

	gl_Position = PIXEL_SPACE_TO_GL_SPACE(instanceVertPos);
	fragInPos = -instanceData.boundsSize / 2 + instanceData.boundsSize * vTexCoords;
	fragInDataOffset = instanceData.contourDataOffset;
}