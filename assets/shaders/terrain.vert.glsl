#include <assets/shaders/transform.h>

out VS_OUT {
	vec3 pos;
	vec2 uv;
} vs_out;

layout(binding = 0) uniform SharedGpuData
{
	mat4 cameraMatrix;
	float modelTransform[8];
	float cameraPositionX;
	float cameraPositionY;
	float cameraPositionZ;
	float windowSizeX;
	float windowSizeY;
};

void main()
{
	Transform modelTr;
	TRANSFORM_LOAD_FROM_ARRAY(modelTr, 0, modelTransform);
	mat4 modelMatrix = TransformToMatrix(modelTr);
	vs_out.pos = (modelMatrix * vec4(VA_Position, 1.0)).xyz;
	vs_out.uv = VA_UV;
}