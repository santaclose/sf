#pragma once

#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	//glm::vec4 color;
	glm::vec2 textureCoord;

	Vertex(float posX, float posY, float posZ);
	Vertex(const glm::vec3& thePosition);
	Vertex(glm::vec3&& thePosition);
};