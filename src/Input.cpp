#include "Input.h"

#include <unordered_map>
#include <iostream>
#include <cstring>

#include <ImGuiController.h>

#define GAMEPAD_BUTTON_COUNT 15

namespace sf::Input {

	int shouldIgnoreMouseDeltaNextFrame = 0;
	bool cursorEnabled = false;

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
	unsigned int character;

	struct GamepadState
	{
		unsigned char buttons[GAMEPAD_BUTTON_COUNT] = { 0U };
		float axes[6] = { 0.0f };
	};
	GamepadState gamepadState;
	unsigned char gamepadButtonsPreviousFrame[GAMEPAD_BUTTON_COUNT];
}

void sf::Input::UpdateFullScreenEnabled()
{
	// there is a frame end before the next frame
	shouldIgnoreMouseDeltaNextFrame = 2;
}

void sf::Input::UpdateCursorEnabled(bool value)
{
	cursorEnabled = value;
	// there is a frame end before the next frame
	// and enabling the cursor takes one more frame when in fullscreen
	shouldIgnoreMouseDeltaNextFrame = value ? 3 : 2;
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

void sf::Input::UpdateCharacter(unsigned int character)
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

	shouldIgnoreMouseDeltaNextFrame--;
}

float sf::Input::MousePosDeltaX()
{
	if (shouldIgnoreMouseDeltaNextFrame > 0) return 0.0f;
	if (cursorEnabled && ImGuiController::HasControl()) return 0.0f;
	return mousePos[0] - lastMousePos[0];
}

float sf::Input::MousePosDeltaY()
{
	if (shouldIgnoreMouseDeltaNextFrame > 0) return 0.0f;
	if (cursorEnabled && ImGuiController::HasControl()) return 0.0f;
	return mousePos[1] - lastMousePos[1];
}

bool sf::Input::MouseButtonDown(int buttonID)
{
	if (cursorEnabled && ImGuiController::HasControl()) return false;
	return mouseButtonsPressing[buttonID];
}

bool sf::Input::MouseButtonUp(int buttonID)
{
	if (cursorEnabled && ImGuiController::HasControl()) return false;
	return mouseButtonsReleasing[buttonID];
}

bool sf::Input::MouseButton(int buttonID)
{
	if (cursorEnabled && ImGuiController::HasControl()) return false;
	return mouseButtons[buttonID];
}

bool sf::Input::MouseScrollUp()
{
	if (cursorEnabled && ImGuiController::HasControl()) return false;
	return mouseScroll[1] == 1.0f;
}

bool sf::Input::MouseScrollDown()
{
	if (cursorEnabled && ImGuiController::HasControl()) return false;
	return mouseScroll[1] == -1.0f;
}

bool sf::Input::KeyDown(int key)
{
	if (cursorEnabled && ImGuiController::HasControl()) return false;
	if (keyStates.find(key) == keyStates.end())
		return false;
	return keyStates[key].pressing;
}

bool sf::Input::KeyUp(int key)
{
	if (cursorEnabled && ImGuiController::HasControl()) return false;
	if (keyStates.find(key) == keyStates.end())
		return false;
	return keyStates[key].releasing;
}

bool sf::Input::Key(int key)
{
	if (cursorEnabled && ImGuiController::HasControl()) return false;
	if (keyStates.find(key) == keyStates.end())
		return false;
	return keyStates[key].isDown;
}

bool sf::Input::KeyRepeat(int key)
{
	if (cursorEnabled && ImGuiController::HasControl()) return false;
	if (keyStates.find(key) == keyStates.end())
		return false;
	return keyStates[key].repeating;
}

bool sf::Input::CharacterInput(unsigned int& character)
{
	if (cursorEnabled && ImGuiController::HasControl()) return false;
	character = Input::character;
	return charInput;
}

void* sf::Input::GetGamepadState()
{
	memcpy(gamepadButtonsPreviousFrame, gamepadState.buttons, GAMEPAD_BUTTON_COUNT);
	return (void*) &gamepadState;
}

bool sf::Input::GamepadButtonDown(int button)
{
	return gamepadState.buttons[button] && !gamepadButtonsPreviousFrame[button];
}

bool sf::Input::GamepadButtonUp(int button)
{
	return !gamepadState.buttons[button] && gamepadButtonsPreviousFrame[button];
}

bool sf::Input::GamepadButton(int button)
{
	return gamepadState.buttons[button];
}

float sf::Input::GamepadLeftStickX()
{
	return gamepadState.axes[GamepadAxes::LeftStickX];
}

float sf::Input::GamepadLeftStickY()
{
	return gamepadState.axes[GamepadAxes::LeftStickY];
}

float sf::Input::GamepadRightStickX()
{
	return gamepadState.axes[GamepadAxes::RightStickX];
}

float sf::Input::GamepadRightStickY()
{
	return gamepadState.axes[GamepadAxes::RightStickY];
}

float sf::Input::GamepadLeftTrigger()
{
	return (gamepadState.axes[GamepadAxes::LeftTrigger] + 1.0f) / 2.0f;
}

float sf::Input::GamepadRightTrigger()
{
	return (gamepadState.axes[GamepadAxes::RightTrigger] + 1.0f) / 2.0f;
}
