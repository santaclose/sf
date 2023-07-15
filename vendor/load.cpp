#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#ifdef SF_USE_OPENGL
#include <backends/imgui_impl_opengl3.cpp>
#endif
#ifdef SF_USE_VULKAN
#include <backends/imgui_impl_vulkan.cpp>
#endif
#include <backends/imgui_impl_glfw.cpp>

#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvgrast.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"