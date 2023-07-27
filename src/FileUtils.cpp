#include "FileUtils.h"

#include <fstream>

bool sf::FileUtils::ReadFileAsString(const std::string& filePath, std::string& outString)
{
	std::ifstream ifs(filePath);
	if (ifs.fail())
		return false;
	outString = std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	return true;
}

bool sf::FileUtils::ReadFileAsBytes(const std::string& filePath, std::vector<char>& outBytes)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		return false;
	size_t fileSize = (size_t)file.tellg();
	outBytes.resize(fileSize);
	file.seekg(0);
	file.read(outBytes.data(), fileSize);
	return true;
}
