#include "Object.h"

void sf::Object::UpdateTransformMatrix()
{
	//std::cout << "Updating model matrix\n";
	m_transformMatrix = glm::translate(glm::mat4(1.0), m_position);

	glm::mat4 rotationMatrix = (glm::mat4) m_rotation;
	m_transformMatrix *= rotationMatrix;

	m_transformMatrix = glm::scale(m_transformMatrix, glm::vec3(m_scale, m_scale, m_scale));

	m_matrixUpdatePending = false;
}

void sf::Object::SetScale(float newScale)
{
	m_scale = newScale;
	m_matrixUpdatePending = true;
}

float sf::Object::GetScale()
{
	return m_scale;
}