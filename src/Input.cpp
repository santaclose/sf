#include "Input.h"

#include <unordered_map>
#include <iostream>

#include <Config.h>
#include <ImGuiController.h>

namespace sf::Input {

	bool mouseButtonsReleasing[3] = { false, false, false };
	bool mouseButtonsPressing[3] = { false, false, false };
	bool mouseButtons[3] = { false, false, false };

	float lastMousePos[2] = { 0.0f, 0.0f };
	float mousePos[2] = { 0.0f, 0.0f };
	bool mousePosDeltaLock = true;

	float mouseScroll[2] = { 0.0f, 0.0f };

	struct KeyState {

		bool pressing = false;
		bool releasing = false;
		bool repeating = false;
		bool isDown = false;
	};

	std::unordered_map<int, KeyState> keyStates;

	bool charInput = false;
	uint32_t character;

	bool cursorCollisionDetected = false;
	void* currentlyHandling = nullptr;
}

void sf::Input::UpdateMouseButtons(int button, int action)
{
	if (button < 0 || button > 2)
		return;

	mouseButtonsReleasing[button] = !action;
	mouseButtonsPressing[button] = action;
	mouseButtons[button] = action;
}

void sf::Input::UpdateMousePosition(double xpos, double ypos)
{
	if (mousePosDeltaLock)
	{
		mousePos[0] = lastMousePos[0] = xpos;
		mousePos[1] = lastMousePos[1] = ypos;
		mousePosDeltaLock = false;
	}
	else
	{
		mousePos[0] = xpos;
		mousePos[1] = ypos;
	}
}

void sf::Input::UpdateKeyboard(int key, int action)
{
	if (keyStates.find(key) == keyStates.end())
		keyStates[key] = KeyState();

	if (action == 1)
	{
		keyStates[key].isDown = true;
		keyStates[key].pressing = true;
	}
	else if (action == 0)
	{
		keyStates[key].isDown = false;
		keyStates[key].releasing = true;
	}
	else
	{
		keyStates[key].repeating = true;
	}
}

void sf::Input::UpdateCharacter(uint32_t character)
{
	Input::character = character;
	Input::charInput = true;
}

void sf::Input::UpdateMouseScroll(float xoffset, float yoffset)
{
	mouseScroll[0] = xoffset;
	mouseScroll[1] = yoffset;
}

void sf::Input::FrameEnd()
{
	mouseButtonsReleasing[0] =
		mouseButtonsPressing[0] =
		mouseButtonsReleasing[1] =
		mouseButtonsPressing[1] =
		mouseButtonsReleasing[2] =
		mouseButtonsPressing[2] = false;
	
	mouseScroll[0] = mouseScroll[1] = 0.0f;

	lastMousePos[0] = mousePos[0];
	lastMousePos[1] = mousePos[1];

	for (auto& pair : keyStates)
	{
		pair.second.pressing =
			pair.second.releasing =
			pair.second.repeating = false;
	}

	charInput = false;

	cursorCollisionDetected = false;
}

float sf::Input::MousePosDeltaX()
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return 0.0f;
	return mousePos[0] - lastMousePos[0];
}

float sf::Input::MousePosDeltaY()
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return 0.0f;
	return mousePos[1] - lastMousePos[1];
}

bool sf::Input::MouseButtonDown(int buttonID)
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return false;
	return mouseButtonsPressing[buttonID];
}

bool sf::Input::MouseButtonUp(int buttonID)
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return false;
	return mouseButtonsReleasing[buttonID];
}

bool sf::Input::MouseButton(int buttonID)
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return false;
	return mouseButtons[buttonID];
}

bool sf::Input::MouseScrollUp()
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return false;
	return mouseScroll[1] == 1.0f;
}

bool sf::Input::MouseScrollDown()
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return false;
	return mouseScroll[1] == -1.0f;
}

bool sf::Input::KeyDown(int key)
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return false;
	if (keyStates.find(key) == keyStates.end())
		return false;
	return keyStates[key].pressing;
}

bool sf::Input::KeyUp(int key)
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return false;
	if (keyStates.find(key) == keyStates.end())
		return false;
	return keyStates[key].releasing;
}

bool sf::Input::Key(int key)
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return false;
	if (keyStates.find(key) == keyStates.end())
		return false;
	return keyStates[key].isDown;
}

bool sf::Input::KeyRepeat(int key)
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return false;
	if (keyStates.find(key) == keyStates.end())
		return false;
	return keyStates[key].repeating;
}

bool sf::Input::CharacterInput(uint32_t & character)
{
	if (Config::GetCursorEnabled() && ImGuiController::HasControl()) return false;
	character = Input::character;
	return charInput;
}