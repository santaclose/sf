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

uniform float voxelSize;
uniform uint bufferSelect;

void main()
{
	fTexCoords = VA_UV;
	vec3 voxelPos = bufferSelect != 0 ? VOXELS_BIG_LOAD_POSITION : VOXELS_SMALL_LOAD_POSITION;
	gl_Position = cameraMatrix * modelMatrix * vec4(voxelPos + VA_Position * voxelSize, 1.0);
}