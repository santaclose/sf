#include "Transform.h"

#include <iostream>

void sf::Transform::LookAt(const glm::vec3& target, const glm::vec3& up)
{
	rotation = glm::quatLookAt(glm::normalize(target - position), up);
}

glm::mat4 sf::Transform::ComputeMatrix() const
{
	glm::mat4 outputMatrix = glm::translate(glm::mat4(1.0), position);
	glm::mat4 rotationMatrix = (glm::mat4)rotation;
	outputMatrix *= rotationMatrix;
	outputMatrix = glm::scale(outputMatrix, glm::vec3(scale, scale, scale));
	return outputMatrix;
}

glm::vec3 sf::Transform::Forward() const
{
	return rotation * glm::vec3(0, 0, -1);
}

glm::vec3 sf::Transform::Right() const
{
	return rotation * glm::vec3(1, 0, 0);
}

glm::vec3 sf::Transform::Up() const
{
	return rotation * glm::vec3(0, 1, 0);
}