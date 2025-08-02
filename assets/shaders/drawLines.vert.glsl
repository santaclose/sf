layout(location = 0) out vec3 fColor;

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

void main()
{
	fColor = VA_Color;
	gl_Position = cameraMatrix * vec4(VA_Position, 1.0);
}