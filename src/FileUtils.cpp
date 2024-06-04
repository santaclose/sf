#include "FileUtils.h"

#include <fstream>
#include <cassert>

void sf::FileUtils::DecompressZip(const std::string& filePath, const char* targetFolderPath)
{
	assert(ExtensionIs(filePath, "zip"));

	std::string targetFolderPathStr;
	if (targetFolderPath == nullptr)
		targetFolderPathStr = RemoveExtension(filePath);
	else
		targetFolderPathStr = std::string(targetFolderPath);

#if SF_PLATFORM_WINDOWS
	std::string commandString = "powershell -Command \"Expand-Archive " + filePath + " -DestinationPath " + targetFolderPathStr + "\"";
#else
	std::string commandString = "unzip " + filePath + " -d \"" + targetFolderPathStr + "\"";
#endif
	system(commandString.c_str());
}

void sf::FileUtils::DownloadFiles(const std::vector<std::string>& urls, const std::string& targetPath, const std::vector<std::string>* targetNames, bool decompressZipFiles)
{
	for (int i = 0; i < urls.size(); i++)
	{
		const std::string& url = urls[i];
		const std::string fileName = targetNames == nullptr ? FileNameFromPath(url) : targetNames->at(i);
		const std::string filePath = CombinePaths(targetPath, fileName);

		std::ifstream f(filePath);
		if (!f.good()) // download only if file doesn't exist
		{
			std::string commandString = "curl -L " + url + " --output " + targetPath + fileName;
			system(commandString.c_str());
			if (decompressZipFiles && ExtensionIs(fileName, "zip"))
				DecompressZip(filePath);
		}
	}
}
