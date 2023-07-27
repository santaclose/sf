#pragma once

#include <vector>
#include <string>

namespace sf::FileUtils
{
	bool ReadFileAsString(const std::string& filePath, std::string& outString);
	bool ReadFileAsBytes(const std::string& filePath, std::vector<char>& outBytes);
}