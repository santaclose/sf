#include "Camera.h"
//#define PI 3.14159265358979323846264338
//#define DEG2RAD 2.0 * PI / 180.0

Camera* Camera::boundCamera = nullptr;

void Camera::UpdateCameraMatrix()
{
	m_cameraMatrix = glm::perspective(glm::radians(m_fieldOfView), m_aspectRatio, m_nearClippingPlane, m_farClippingPlane);

	glm::mat4 rotationMatrix = (glm::mat4) glm::conjugate(m_rotation);

	m_cameraMatrix *= rotationMatrix;
	m_cameraMatrix = glm::translate(m_cameraMatrix, -m_position);

	m_matrixUpdatePending = false;
}

Camera::Camera(float aspectRatio, float fieldOfView, float nearClippingPlane, float farClippingPlane)
{
	if (boundCamera == nullptr)
		boundCamera = this;

	m_position = glm::vec3(0.0, 0.0, 0.0);
	m_rotation = glm::fquat(1.0, 0.0, 0.0, 0.0);
	m_nearClippingPlane = nearClippingPlane;
	m_farClippingPlane = farClippingPlane;
	m_fieldOfView = fieldOfView;
	m_aspectRatio = aspectRatio;

	m_cameraMatrix = glm::mat4();
}

void Camera::Bind()
{
	boundCamera = this;
}

void Camera::SendMatrixToShader(Shader& theShader)//updateshadermatrix
{
	if (boundCamera->m_matrixUpdatePending)
		boundCamera->UpdateCameraMatrix();
	theShader.SetUniformMatrix4fv("cameraMatrix", &(boundCamera->m_cameraMatrix[0][0]));
}