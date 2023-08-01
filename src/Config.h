#pragma once

#include <string>
#include <glm/glm.hpp>

namespace sf::Config
{
	void LoadFromFile(const std::string& filePath);

	const std::string& GetName();
	const glm::uvec2& GetWindowSize();
	const uint32_t GetMsaaCount();
	const glm::vec4& GetClearColor();
	const bool& GetFullscreen();
	const bool& GetImGuiBarEnabled();
	const bool& GetCursorEnabled();
	const bool& GetVsyncEnabled();
};