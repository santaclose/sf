#include "Camera.h"
//#define PI 3.14159265358979323846264338
//#define DEG2RAD 2.0 * PI / 180.0

Camera* Camera::boundCamera = nullptr;

void Camera::ComputeMatrices()
{
	boundCamera->m_projectionMatrix = glm::perspective(glm::radians(boundCamera->m_fieldOfView), boundCamera->m_aspectRatio, boundCamera->m_nearClippingPlane, boundCamera->m_farClippingPlane);

	boundCamera->m_viewMatrix = (glm::mat4) glm::conjugate(boundCamera->m_rotation);
	boundCamera->m_viewMatrix = glm::translate(boundCamera->m_viewMatrix, -boundCamera->m_position);

	/*
	boundCamera->m_cameraMatrix = glm::perspective(glm::radians(boundCamera->m_fieldOfView), boundCamera->m_aspectRatio, boundCamera->m_nearClippingPlane, boundCamera->m_farClippingPlane);
	boundCamera->m_cameraMatrix *= (glm::mat4) glm::conjugate(boundCamera->m_rotation);
	boundCamera->m_cameraMatrix = glm::translate(boundCamera->m_cameraMatrix, -boundCamera->m_position);*/

	boundCamera->m_cameraMatrix = boundCamera->m_projectionMatrix * boundCamera->m_viewMatrix;
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

	m_cameraMatrix = m_viewMatrix = m_projectionMatrix = glm::mat4();
}

void Camera::Bind()
{
	boundCamera = this;
}

const glm::mat4& Camera::GetMatrix()
{
	return boundCamera->m_cameraMatrix;
}

const glm::mat4& Camera::GetViewMatrix()
{
	return boundCamera->m_viewMatrix;
}

const glm::mat4& Camera::GetProjectionMatrix()
{
	return boundCamera->m_projectionMatrix;
}
const glm::vec3& Camera::GetPosition()
{
	return boundCamera->m_position;
}
/*
void Camera::SendMatrixToShader(Shader& theShader)//updateshadermatrix
{
	if (boundCamera->m_matrixUpdatePending)
		boundCamera->UpdateCameraMatrix();
	theShader.SetUniformMatrix4fv("cameraMatrix", &(boundCamera->m_cameraMatrix[0][0]));
	theShader.SetUniform3fv("camPos", &(boundCamera->m_position[0]));
}*/