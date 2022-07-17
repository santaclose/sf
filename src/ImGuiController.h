#pragma once

#include <GLFW/glfw3.h>

namespace sf::ImGuiController
{
	void Setup(GLFWwindow* window);
	bool HasControl();
	void Tick(float deltaTime);
}