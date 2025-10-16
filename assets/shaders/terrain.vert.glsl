out VS_OUT {
	vec3 pos;
	vec2 uv;
} vs_out;

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

void main()
{
	vs_out.pos = (modelMatrix * vec4(VA_Position, 1.0)).xyz;
	vs_out.uv = VA_UV;
}