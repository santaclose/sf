#pragma once

#include <string>
#include <glm/glm.hpp>
#include <Alignment.h>

namespace sf {

	struct Text
	{
		const char* fontPath;
		const char* string;
		glm::vec4 color;
		float size = 1.0f;
		int alignmentH = ALIGNMENT_LEFT;
		int alignmentV = ALIGNMENT_TOP;
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