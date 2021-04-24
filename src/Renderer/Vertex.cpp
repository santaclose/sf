#include "Vertex.h"

sf::Vertex::Vertex(float posX, float posY, float posZ)
{
	position = glm::vec3(posX, posY, posZ);
	normal = glm::vec3(0, 0, 0);
	textureCoord = glm::vec2(0, 0);
	extraData = glm::vec2(0, 0);
}

sf::Vertex::Vertex(const glm::vec3& thePosition)
{
	position = thePosition;
	normal = glm::vec3(0, 0, 0);
	textureCoord = glm::vec2(0, 0);
	extraData = glm::vec2(0, 0);
}

sf::Vertex::Vertex(glm::vec3&& thePosition)
{
	position = thePosition;
	normal = glm::vec3(0, 0, 0);
	textureCoord = glm::vec2(0, 0);
	extraData = glm::vec2(0, 0);
}

