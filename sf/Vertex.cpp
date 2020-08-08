#include "Vertex.h"

Vertex::Vertex(float posX, float posY, float posZ)
{
	position = glm::vec3(posX, posY, posZ);
	normal = glm::vec3(0, 0, 0);
	//color = glm::vec4(1.0, 0.0, 1.0, 1.0);
	textureCoord = glm::vec2(0, 0);
}

Vertex::Vertex(const glm::vec3& thePosition)
{
	position = thePosition;
	normal = glm::vec3(0, 0, 0);
	//color = glm::vec4(1.0, 0.0, 1.0, 1.0);
	textureCoord = glm::vec2(0, 0);
}

Vertex::Vertex(glm::vec3&& thePosition)
{
	position = thePosition;
	normal = glm::vec3(0, 0, 0);
	//color = glm::vec4(1.0, 0.0, 1.0, 1.0);
	textureCoord = glm::vec2(0, 0);
}

