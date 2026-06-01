
#define RENDER_TARGET_SIZE_VEC vec2(renderTargetSizeX, renderTargetSizeY)
#define CAMERA_POSITION_VEC vec3(cameraPositionX, cameraPositionY, cameraPositionZ)
#define PIXEL_SPACE_TO_GL_SPACE(vecin) vec4(vecin.x / renderTargetSizeX * 2.0 - 1.0, -vecin.y / renderTargetSizeY * 2.0 + 1.0, 0.0, 1.0)