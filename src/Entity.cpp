#include "Entity.h"
#include <iostream>

void sf::Entity::SetPosition(const glm::vec3& newPosition)
{
	m_position = newPosition;
	m_matrixUpdatePending = true;
}
void sf::Entity::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
	m_matrixUpdatePending = true;
}

void sf::Entity::SetRotation(const glm::fquat& newRotation)
{
	m_rotation = newRotation;
	m_matrixUpdatePending = true;
}

void sf::Entity::SetRotation(const glm::vec3& newRotation)
{
	m_rotation = glm::fquat(newRotation);
	m_matrixUpdatePending = true;
}
void sf::Entity::SetRotation(float x, float y, float z)
{
	m_rotation = glm::fquat(glm::vec3(x, y, z));
	m_matrixUpdatePending = true;
}

void sf::Entity::LookAt(const glm::vec3& target, const glm::vec3& up)
{
	m_rotation = /*glm::conjugate(*/glm::quatLookAt(glm::normalize(target - m_position), up)/*)*/;
	m_matrixUpdatePending = true;
}

const glm::vec3& sf::Entity::GetPosition()
{
	return m_position;
}

const glm::fquat& sf::Entity::GetRotation()
{
	return m_rotation;
}

glm::vec3 sf::Entity::Forward()
{
	return m_rotation * glm::vec3(0, 0, -1);
}

glm::vec3 sf::Entity::Right()
{
	return m_rotation * glm::vec3(1, 0, 0);
}

glm::vec3 sf::Entity::Up()
{
	return m_rotation * glm::vec3(0, 1, 0);
}