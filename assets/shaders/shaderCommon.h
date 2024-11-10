
#if VULKAN

layout(binding = 0) uniform SharedGpuData
{
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
} sgd;

layout(push_constant) uniform constants
{
	mat4 modelMatrix;
} pc;

#define OBJECT_MATRIX pc.modelMatrix
#define CAMERA_MATRIX sgd.cameraMatrix
#define CAMERA_POSITION sgd.cameraPosition
#define SCREEN_SPACE_MATRIX sgd.screenSpaceMatrix

#define INSTANCE_INDEX gl_InstanceIndex

#else // OPENGL


layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

#define OBJECT_MATRIX modelMatrix
#define CAMERA_MATRIX cameraMatrix
#define CAMERA_POSITION cameraPosition
#define SCREEN_SPACE_MATRIX screenSpaceMatrix

#define INSTANCE_INDEX gl_InstanceID

#endif