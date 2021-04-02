#include "Camera.h"

namespace sf {
	Camera* Camera::boundCamera = nullptr;
}

void sf::Camera::ComputeMatrices()
{
	if (m_specs.perspective)
	{
		m_projectionMatrix = glm::perspective(
			m_specs.fieldOfView,
			m_specs.aspectRatio,
			m_specs.nearClippingPlane,
			m_specs.farClippingPlane);
	}
	else
	{
		if (m_specs.aspectRatio >= 1.0)
			m_projectionMatrix = glm::ortho(
				-m_specs.aspectRatio / 2.0f * m_specs.orthographicScale,
				m_specs.aspectRatio / 2.0f * m_specs.orthographicScale,
				-0.5f * m_specs.orthographicScale,
				0.5f * m_specs.orthographicScale,
				m_specs.nearClippingPlane,
				m_specs.farClippingPlane);
		else
			m_projectionMatrix = glm::ortho(
				-0.5f * m_specs.orthographicScale,
				0.5f * m_specs.orthographicScale,
				-1.0f / m_specs.aspectRatio / 2.0f * m_specs.orthographicScale,
				1.0f / m_specs.aspectRatio / 2.0f * m_specs.orthographicScale,
				m_specs.nearClippingPlane,
				m_specs.farClippingPlane);
	}

	m_viewMatrix = (glm::mat4) glm::conjugate(m_rotation);
	m_viewMatrix = glm::translate(m_viewMatrix, -m_position);

	m_cameraMatrix = m_projectionMatrix * m_viewMatrix;
}

sf::Camera::Camera(const CameraSpecs& specs)
{
	if (boundCamera == nullptr)
		boundCamera = this;

	m_position = glm::vec3(0.0, 0.0, 0.0);
	m_rotation = glm::fquat(1.0, 0.0, 0.0, 0.0);

	m_specs = specs;

	//m_cameraMatrix = m_viewMatrix = m_projectionMatrix = glm::mat4(1.0f);
}

void sf::Camera::Bind()
{
	boundCamera = this;
}

const glm::mat4& sf::Camera::GetMatrix()
{
	return boundCamera->m_cameraMatrix;
}

const glm::mat4& sf::Camera::GetViewMatrix()
{
	return boundCamera->m_viewMatrix;
}

const glm::mat4& sf::Camera::GetProjectionMatrix()
{
	return boundCamera->m_projectionMatrix;
}