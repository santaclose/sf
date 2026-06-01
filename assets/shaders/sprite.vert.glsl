layout(location = 4) out vec2 fTexCoords;

#include <assets/shaders/shared.h>

layout(binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	float cameraPositionX;
	float cameraPositionY;
	float cameraPositionZ;
	float renderTargetSizeX;
	float renderTargetSizeY;
};

void main()
{
	fTexCoords = VA_UV;
	gl_Position = PIXEL_SPACE_TO_GL_SPACE(VA_Position);
}