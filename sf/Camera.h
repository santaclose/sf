#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Shader.h"
#include "Entity.h"

class Camera : public Entity
{
private:
	glm::mat4 m_projectionMatrix;
	glm::mat4 m_viewMatrix;
	glm::mat4 m_cameraMatrix;

	float m_nearClippingPlane;
	float m_farClippingPlane;
	float m_fieldOfView;
	float m_aspectRatio;

public:
	static void ComputeMatrices();

	Camera(float aspectRatio, float fieldOfView = 90.0, float nearClippingPlane = 0.1, float farClippingPlane = 100.0);
	void Bind();

	static Camera* boundCamera;
	static const glm::mat4& GetMatrix();
	static const glm::mat4& GetViewMatrix();
	static const glm::mat4& GetProjectionMatrix();

	static const glm::vec3& GetPosition();

	//static void SendMatrixToShader(Shader& theShader);
};

