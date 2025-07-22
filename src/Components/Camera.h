#pragma once

#include <glm/glm.hpp>

namespace sf {

	struct Camera
	{
		bool perspective = true;
		float orthographicScale = 5.0f;
		float fieldOfView = glm::radians(90.0f);
		float nearClippingPlane = 0.1f;
		float farClippingPlane = 1000.0f;
	};
}