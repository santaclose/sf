#include <assets/shaders/particle.h>

layout(location = 4) out vec2 fTexCoords;


layout(binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	float cameraPositionX;
	float cameraPositionY;
	float cameraPositionZ;
	float windowSizeX;
	float windowSizeY;
};

layout (std430, binding = 1) buffer PerVoxelBuffer
{
	float perVoxelData[];
};

uniform float voxelSize;

void main()
{
	fTexCoords = VA_UV;
	vec3 voxelPos = vec3(perVoxelData[gl_InstanceID * 3 + 0], perVoxelData[gl_InstanceID * 3 + 1], perVoxelData[gl_InstanceID * 3 + 2]);
	gl_Position = cameraMatrix * modelMatrix * vec4(voxelPos + VA_Position * voxelSize, 1.0);
}