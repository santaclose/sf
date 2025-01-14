#pragma once

#include <vector>
#include <string>
#include <cstring>

namespace sf::FileUtils
{
	inline std::string CombinePaths(const std::string& a, const std::string& b) { return (a[a.length() - 1] == '/' || a[a.length() - 1] == '\\') ? a + b : a + '/' + b; }
	inline std::string RemoveExtension(const std::string& filePath) { return filePath.substr(0, filePath.find_last_of('.')); }
	inline const char* ExtensionFromPath(const std::string& filePath) { return filePath.c_str() + (filePath.find_last_of('.') + 1); }
	inline const char* FileNameFromPath(const std::string& filePath) { return filePath.c_str() + std::max(filePath.find_last_of('/') + 1, filePath.find_last_of('\\') + 1); }
	inline bool ExtensionIs(const std::string& filePath, const char* extension) { return strcmp(ExtensionFromPath(filePath), extension) == 0; }

	void DecompressZip(const std::string& filePath, const char* targetFolderPath = nullptr);
	void DownloadFiles(const std::vector<std::string>& urls, const std::string& targetPath, const std::vector<std::string>* targetNames = nullptr, bool decompressZipFiles = true);
}