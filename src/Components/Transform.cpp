#include "Transform.h"

#include <iostream>

void sf::Transform::UpdateTransformMatrix() const
{
	matrix = glm::translate(glm::mat4(1.0), position);
	glm::mat4 rotationMatrix = (glm::mat4)rotation;
	matrix *= rotationMatrix;
	matrix = glm::scale(matrix, glm::vec3(scale, scale, scale));
	matrixUpdatePending = false;
}

void sf::Transform::SetPosition(const glm::vec3& newPosition)
{
	position = newPosition;
	matrixUpdatePending = true;
}
void sf::Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
	matrixUpdatePending = true;
}

void sf::Transform::SetRotation(const glm::fquat& newRotation)
{
	rotation = newRotation;
	matrixUpdatePending = true;
}

void sf::Transform::SetRotation(const glm::vec3& newRotation)
{
	rotation = glm::fquat(newRotation);
	matrixUpdatePending = true;
}
void sf::Transform::SetRotation(float x, float y, float z)
{
	rotation = glm::fquat(glm::vec3(x, y, z));
	matrixUpdatePending = true;
}

void sf::Transform::SetScale(float newScale)
{
	scale = newScale;
	matrixUpdatePending = true;
}

void sf::Transform::LookAt(const glm::vec3& target, const glm::vec3& up)
{
	rotation = glm::quatLookAt(glm::normalize(target - position), up);
	matrixUpdatePending = true;
}

const glm::vec3& sf::Transform::GetPosition() const
{
	return position;
}

const glm::fquat& sf::Transform::GetRotation() const
{
	return rotation;
}

float sf::Transform::GetScale() const
{
	return scale;
}

const glm::mat4& sf::Transform::GetMatrix() const
{
	if (matrixUpdatePending)
		UpdateTransformMatrix();
	return matrix;
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