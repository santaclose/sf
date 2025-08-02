#pragma once

#include <Renderer/GlShader.h>
#include <Renderer/GlCubemap.h>

namespace sf {

	class GlSkybox
	{
		static bool generated;
		static uint32_t gl_VAO;
		static uint32_t gl_VBO;
		static float cubeVertices[];
		static GlShader shader;
		static GlCubemap* cubemap;
		static BufferLayout vertexBufferLayout;

	public:
		static void SetCubemap(GlCubemap* cubemap);
		static void SetUseExposure(bool value);
		static void SetExposure(float value);
		static void Draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
	};
}