#version 430 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoords;

out vec2 fragInPos;
flat out int fragInDataOffset;

struct InstanceData
{
	vec2 pos;
	vec2 size;
	float fontSize;
	int dataOffset;
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
layout (std430, binding = 2) buffer bezierSSBO
{
	vec2 BezierData[];
};
layout (std430, binding = 3) buffer metadataSSBO
{
	int GlyphMetaData[];
};

uniform vec2 globalOffset = vec2(0.0, 0.0);

void main()
{
	InstanceData instanceData = PerInstanceData[gl_InstanceID];

	vec2 offset = instanceData.pos + globalOffset;
	vec2 instanceVertPos = vPosition.xy * instanceData.size * instanceData.fontSize + offset;

	gl_Position = vec4(instanceVertPos.x, instanceVertPos.y, 0.0, 1.0);
	fragInPos = -instanceData.size / 2 + instanceData.size * vTexCoords;
	fragInDataOffset = instanceData.dataOffset;
}