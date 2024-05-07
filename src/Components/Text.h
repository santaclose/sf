#pragma once

#include <string>
#include <glm/glm.hpp>

namespace sf {

	struct Text
	{
		const char* fontPath;
		const char* string;
		glm::vec4 color;
		float size = 1.0f;
		Text() = default;
		inline Text(const char* fontPath, const char* string, const glm::vec4& color, float size)
		{
			this->fontPath = fontPath;
			this->string = string;
			this->color = color;
			this->size = size;
		}
	};
}