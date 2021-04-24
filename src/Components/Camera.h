#pragma once

#include <glm/glm.hpp>

namespace sf {

	struct Camera// : public Component
	{
		static int counter;

		int id;
		bool perspective = true;
		float orthographicScale = 5.0;
		float fieldOfView = glm::radians(90.0f);
		float nearClippingPlane = 0.1;
		float farClippingPlane = 1000.0;

		Camera();
	};
}