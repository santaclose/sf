#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace sf::Renderer
{
	struct Framebuffer
	{
		const uint32_t* id;
		const glm::uvec2* size;
		bool hasDepth;
	};
	struct RenderTarget
	{
		Framebuffer framebuffer;
		glm::vec2 topLeftOrigin;
		glm::ivec2 topLeftOffset;
		glm::vec2 bottomRightOrigin;
		glm::ivec2 bottomRightOffset;
		glm::vec3 clearColor;
	};
}