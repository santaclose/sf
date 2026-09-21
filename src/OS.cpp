#include "OS.h"

#include <portable-file-dialogs.h>
#include <imgui.h>

std::vector<std::string> sf::OS::OpenFileDialog(const char* extensions, const char* title, const char* defaultPath, const char* extensionDropDownTitle)
{
	if (title == nullptr)
		title = "Open File";
	if (defaultPath == nullptr)
		defaultPath = ".";
	if (extensionDropDownTitle == nullptr)
		extensionDropDownTitle = extensions;
	return pfd::open_file(title, defaultPath == nullptr ? "" : defaultPath, { extensionDropDownTitle, extensions }).result();
}

std::string sf::OS::SaveFileDialog(const char* extension, const char* title, const char* defaultPath, const char* extensionDropDownTitle)
{
	if (title == nullptr)
		title = "Save File";
	if (defaultPath == nullptr)
		defaultPath = ".";
	if (extensionDropDownTitle == nullptr)
		extensionDropDownTitle = extension;
	return pfd::save_file(title, defaultPath == nullptr ? "" : defaultPath, { extensionDropDownTitle, extension }).result();
}

const char* sf::OS::GetClipboardText()
{
	return ImGui::GetClipboardText();
}

void sf::OS::SetClipboardText(const char* text)
{
	ImGui::SetClipboardText(text);
}

void sf::OS::ShowNotification(const char* message, const char* title)
{
    pfd::notify(title == nullptr ? "" : title, message);
}

sf::OS::Choice sf::OS::YesNoMessageBox(const char* message, const char* title)
{
    pfd::button choice = pfd::message(title == nullptr ? "" : title, message, pfd::choice::yes_no).result();
    return (Choice) choice;
}