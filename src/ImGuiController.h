#pragma once

#include <GLFW/glfw3.h>

namespace sf::ImGuiController
{
	void Initialize(GLFWwindow* window);
	void Terminate();
	bool HasControl();
	void Tick(float deltaTime);
	void OnResize();
}