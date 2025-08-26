
#define WINDOW_SIZE_VEC vec2(windowSizeX, windowSizeY)
#define CAMERA_POSITION_VEC vec3(cameraPositionX, cameraPositionY, cameraPositionZ)
#define PIXEL_SPACE_TO_GL_SPACE(vecin) vec4(vecin.x / windowSizeX * 2.0 - 1.0, -vecin.y / windowSizeY * 2.0 + 1.0, 0.0, 1.0)