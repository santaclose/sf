#pragma once

#include <string>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace sf::Config
{
	void LoadFromFile(const std::string& filePath);
	void UpdateWindow(GLFWwindow* window);
	void UpdateWindowSize(int width, int height);

	void SetImGuiBarEnabled(bool value);
	void SetFullscreen(bool value);
	void SetCursorEnabled(bool value);
	void SetVsyncEnabled(bool value);

	const std::string& GetName();
	const glm::uvec2& GetWindowSize();
	const uint32_t GetMsaaCount();
	const glm::vec4& GetClearColor();
	const bool& GetFullscreen();
	const bool& GetImGuiBarEnabled();
	const bool& GetCursorEnabled();
	const bool& GetVsyncEnabled();
};