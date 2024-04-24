#pragma once

#include <string>
#include <glm/glm.hpp>

namespace sf
{
	struct GameInitializationData
	{
		GameInitializationData() = default;
		GameInitializationData(const std::string& filePath);
		std::string windowTitle = "sfgame";
		glm::uvec2 windowSize = { 1280, 720 };
		uint32_t windowMsaaCount = 4;
		glm::vec3 windowClearColor = { 1.0f, 1.0f, 1.0f };
		bool windowFullscreenEnabled = false;
		bool windowToolBarEnabled = true;
		bool windowCursorEnabled = true;
		bool windowVsyncEnabled = true;
	};
}