#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Shader.h"
#include "Entity.h"

class Camera : public Entity
{
	glm::mat4 m_cameraMatrix;

	//glm::vec3 m_position;
	//glm::fquat m_rotation;
	//glm::vec3 m_rotation;

	float m_nearClippingPlane;
	float m_farClippingPlane;
	float m_fieldOfView;
	float m_aspectRatio;

	void UpdateCameraMatrix();

public:
	Camera(float aspectRatio, float fieldOfView = 90.0, float nearClippingPlane = 0.1, float farClippingPlane = 100.0);
	void SendMatrixToShader(Shader& theShader);
	//void SetRotation(glm::vec3& newRotation);
	//void SetPosition(glm::vec3& newPosition);
};

