#pragma once

#define SENSITIVITY 0.007

namespace Input
{
	double mousePosDelta[2];
	double lastMousePos[2];

	void OnMouseMoved(GLFWwindow* window, double xpos, double ypos)
	{
		using namespace glm;

		mousePosDelta[0] = xpos - lastMousePos[0];
		mousePosDelta[1] = ypos - lastMousePos[1];

		//std::cout << xpos << std::endl;
		//std::cout << ypos << std::endl;
		targetShipRotation *= fquat(vec3(mousePosDelta[1] * SENSITIVITY, 0.0, -mousePosDelta[0] * SENSITIVITY));
		//targetShipRotation *= fquat(vec3(0.0, 0.0, mousePosDelta[0] * SENSITIVITY));
		//ter.z += xpos * SENSITIVITY;
		//shipRotation.x += ypos * SENSITIVITY;

		lastMousePos[0] = xpos;
		lastMousePos[1] = ypos;
	}

	void OnScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		std::cout << yoffset << std::endl;
		shipSpeed += yoffset; // / *SENSITIVITY;
	}
}