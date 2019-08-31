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

		//-------------------//
		targetModelRotation += glm::vec3(mousePosDelta[1], mousePosDelta[0], 0.0) * SENSITIVITY;
		//-------------------//

		lastMousePos[0] = xpos;
		lastMousePos[1] = ypos;
	}

	void OnScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		currentZoom -= yoffset * .1;
	}

	void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			switch (key)
			{
			case GLFW_KEY_M:
				glPolygonMode(GL_FRONT, GL_POINT);
				break;
			case GLFW_KEY_N:
				glPolygonMode(GL_FRONT, GL_LINE);
				break;
			case GLFW_KEY_B:
				glPolygonMode(GL_FRONT, GL_FILL);
				break;
			}
		}
	}
}