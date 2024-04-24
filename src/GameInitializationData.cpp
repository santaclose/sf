#include "GameInitializationData.h"

#include <iostream>
#include <fstream>
#include <json.hpp>

sf::GameInitializationData::GameInitializationData(const std::string& filePath)
{
	std::ifstream i(filePath);
	if (!i.good())
	{
		std::cout << "[GameInitializationData] Config file not found\n";
		return;
	}
	nlohmann::json j;
	i >> j;
	if (j.find("name") != j.end())
		windowTitle = j["name"];
	if (j.find("windowWidth") != j.end() && j.find("windowHeight") != j.end())
		windowSize = { j["windowWidth"], j["windowHeight"] };
	if (j.find("msaaCount") != j.end())
		windowMsaaCount = j["msaaCount"];
	if (j.find("clearColorR") != j.end() && j.find("clearColorG") != j.end() && j.find("clearColorB") != j.end())
		windowClearColor = { j["clearColorR"], j["clearColorG"], j["clearColorB"] };
	if (j.find("fullscreen") != j.end())
		windowFullscreenEnabled = j["fullscreen"];
	if (j.find("toolBarEnabled") != j.end())
		windowToolBarEnabled = j["toolBarEnabled"];
	if (j.find("cursorEnabled") != j.end())
		windowCursorEnabled = j["cursorEnabled"];
	if (j.find("vsyncEnabled") != j.end())
		windowVsyncEnabled = j["vsyncEnabled"];
	std::cout << "[GameInitializationData] Config file loaded\n";
}
