#pragma once

#include <glm/glm.hpp>

namespace sf {

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		glm::vec2 textureCoord;
		glm::vec2 extraData;

		Vertex(float posX, float posY, float posZ);
		Vertex(const glm::vec3& thePosition);
		Vertex(glm::vec3&& thePosition);

		Vertex() = default;
	};
}