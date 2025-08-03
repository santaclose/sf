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

layout (std430, binding = 1) buffer _VOXEL_BUFFER
{
	float VOXEL_BUFFER[];
};

uniform float voxelSize;

void main()
{
	fTexCoords = VA_UV;
	gl_Position = cameraMatrix * modelMatrix * vec4(LOAD_VOXEL_POSITION + VA_Position * voxelSize, 1.0);
}