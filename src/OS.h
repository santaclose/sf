#pragma once

#include <string>
#include <vector>

namespace sf::OS
{
	enum class Choice { Cancel = -1, Ok, Yes, No, Abort, Retry, Ignore };

	std::vector<std::string> OpenFileDialog(const char* extensions, const char* title = nullptr, const char* defaultPath = nullptr, const char* extensionDropDownTitle = nullptr);
	std::string SaveFileDialog(const char* extension, const char* title = nullptr, const char* defaultPath = nullptr, const char* extensionDropDownTitle = nullptr);
	const char* GetClipboardText();
	void SetClipboardText(const char* text);
	void ShowNotification(const char* message, const char* title = nullptr);
	Choice YesNoMessageBox(const char* message, const char* title = nullptr);
}