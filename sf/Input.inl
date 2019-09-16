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
		Game::OnMouseMove(xpos, ypos, mousePosDelta);
		//-------------------//

		lastMousePos.x = xpos;
		lastMousePos.y = ypos;
	}

	void OnScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		//-------------------//
		Game::OnScroll(xoffset, yoffset);
		//-------------------//
	}

	void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		//-------------------//
		Game::OnKey(key, action);
		//-------------------//
	}
}