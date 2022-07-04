#pragma once

#include <glm/glm.hpp>

namespace sf
{
	struct ScreenCoordinates
	{
		// for origin, 0.0, 0.0 is top left corner and 1.0, 1.0 is bottom right corner
		glm::vec2 origin = { 0.0f, 0.0f };
		glm::ivec2 offset = { 0, 0 };
	};
}