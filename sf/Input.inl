#pragma once

#define SENSITIVITY 0.007

namespace Input
{
	glm::vec2 mousePosDelta;
	glm::vec2 lastMousePos;
	bool firstFrame = true;

	void OnMouseMoved(GLFWwindow* window, double xpos, double ypos)
	{
		if (firstFrame) // first frame
		{
			mousePosDelta.x = 0;
			mousePosDelta.y = 0;
			firstFrame = false;
		}
		else
		{
			mousePosDelta.x = xpos - lastMousePos.x;
			mousePosDelta.y = ypos - lastMousePos.y;
		}

		//-------------------//
		targetModelRotation.y += mousePosDelta.x * SENSITIVITY;
		targetModelRotation.x += mousePosDelta.y * SENSITIVITY;
		//-------------------//

		lastMousePos.x = xpos;
		lastMousePos.y = ypos;
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