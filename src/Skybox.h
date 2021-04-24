#pragma once

#include <Renderer/Shader.h>
#include <Renderer/Cubemap.h>

namespace sf {

	class Skybox
	{
		static bool generated;
		static unsigned int gl_VAO;
		static unsigned int gl_VBO;
		static float cubeVertices[];
		static Shader shader;
		static Cubemap* cubemap;

	public:
		static void SetCubemap(Cubemap* cubemap);
		static void SetUseExposure(bool value);
		static void SetExposure(float value);
		static void Draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
	};
}