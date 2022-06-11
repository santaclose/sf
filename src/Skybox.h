#pragma once

#include <Renderer/GlShader.h>
#include <Renderer/GlCubemap.h>

namespace sf {

	class Skybox
	{
		static bool generated;
		static unsigned int gl_VAO;
		static unsigned int gl_VBO;
		static float cubeVertices[];
		static GlShader shader;
		static GlCubemap* cubemap;

	public:
		static void SetCubemap(GlCubemap* cubemap);
		static void SetUseExposure(bool value);
		static void SetExposure(float value);
		static void Draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
	};
}