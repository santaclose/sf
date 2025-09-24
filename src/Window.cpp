#include "Window.h"

#ifdef SF_PLATFORM_WINDOWS
#include <windows.h>
#endif

#ifdef SF_USE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#ifdef SF_USE_VULKAN
#ifdef SF_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>
#endif
#ifdef SF_USE_VULKAN
#include <vulkan/vulkan.h>
#endif

#include <iostream>
#include <cassert>

#include <Input.h>

std::unordered_map<GLFWwindow*, sf::Window*> sf::Window::windowMappingForCallbacks;

sf::Window::Window(const Game::InitData& gameInitData)
{
	if (!glfwInit())
	{
		std::cout << "[Window] Failed to create window\n";
		return;
	}
	title = gameInitData.windowTitle;
	size = gameInitData.windowSize;
	msaaCount = gameInitData.msaaCount;
	fullscreenEnabled = gameInitData.windowFullscreenEnabled;
	toolBarEnabled = gameInitData.toolBarEnabled;
	cursorEnabled = gameInitData.windowCursorEnabled;
	vsyncEnabled = gameInitData.vsyncEnabled;


#ifdef SF_USE_VULKAN
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
#ifdef SF_USE_OPENGL
	glfwWindowHint(GLFW_SAMPLES, msaaCount);
#ifdef SF_DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
#endif

	windowHandle = glfwCreateWindow(size.x, size.y, title.c_str(), fullscreenEnabled ? glfwGetPrimaryMonitor() : NULL, NULL);
	if (!windowHandle)
	{
		glfwTerminate();
		std::cout << "[Window] Failed to create window\n";
		return;
	}

#if SF_USE_VULKAN
	if (!glfwVulkanSupported())
	{
		std::cout << "[Window] Vulkan not supported\n";
		return;
	}
#endif

	glfwMakeContextCurrent(windowHandle);
	glfwSetInputMode(windowHandle, GLFW_CURSOR, cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

	/* INPUT BINDINGS */
	glfwSetCursorPosCallback(windowHandle, CursorPositionCallback);
	glfwSetMouseButtonCallback(windowHandle, MouseButtonCallback);
	glfwSetKeyCallback(windowHandle, KeyCallback);
	glfwSetCharCallback(windowHandle, CharacterCallback);
	glfwSetScrollCallback(windowHandle, ScrollCallback);
	GLFWgamepadstate state;

	glfwSetFramebufferSizeCallback(windowHandle, WindowResizeCallback);
	glfwSwapInterval(vsyncEnabled);

	windowMappingForCallbacks[windowHandle] = this;
	Input::UpdateCursorEnabled(cursorEnabled);

	this->title = title;
	this->size = size;
	this->fullscreenEnabled = fullscreenEnabled;
	this->cursorEnabled = cursorEnabled;
	this->vsyncEnabled = vsyncEnabled;
}

sf::Window::~Window()
{
	glfwDestroyWindow(windowHandle);
}

void sf::Window::PollEvents()
{
	glfwPollEvents();
	glfwGetGamepadState(GLFW_JOYSTICK_1, (GLFWgamepadstate*)Input::GetGamepadState());
}

void sf::Window::SwapBuffers()
{
	glfwSwapBuffers(windowHandle);
}

double sf::Window::GetTime()
{
	return glfwGetTime();
}

bool sf::Window::ShouldClose()
{
	return glfwWindowShouldClose(windowHandle);
}

void sf::Window::SetFullScreenEnabled(bool enabled)
{
	if (fullscreenEnabled == enabled)
		return;

	fullscreenEnabled = enabled;
	static int xpos, ypos, width, height, monitorCount, monitorxpos, monitorypos;
	if (fullscreenEnabled)
	{
		// backup window position and window size
		glfwGetWindowPos(windowHandle, &xpos, &ypos);
		glfwGetWindowSize(windowHandle, &width, &height);

		// get resolution of monitor
		GLFWmonitor* targetMonitor = glfwGetPrimaryMonitor();
		GLFWmonitor** monitorList = glfwGetMonitors(&monitorCount);
		float minMonitorDistance = std::numeric_limits<float>::infinity();
		for (int i = 0; i < monitorCount; i++)
		{
			glfwGetMonitorPos(monitorList[i], &monitorxpos, &monitorypos);
			float distance = glm::distance(glm::vec2(xpos, ypos), glm::vec2(monitorxpos, monitorypos));
			if (distance < minMonitorDistance)
			{
				minMonitorDistance = distance;
				targetMonitor = monitorList[i];
			}
		}
		const GLFWvidmode* mode = glfwGetVideoMode(targetMonitor);

		// switch to full screen
		glfwSetWindowMonitor(windowHandle, targetMonitor, 0, 0, mode->width, mode->height, 0);
	}
	else
	{
		// restore last window size and position
		glfwSetWindowMonitor(windowHandle, nullptr, xpos, ypos, width, height, 0);
	}
	Input::UpdateFullScreenEnabled();
}

void sf::Window::SetCursorEnabled(bool enabled)
{
	if (cursorEnabled == enabled)
		return;
	cursorEnabled = enabled;
	glfwSetInputMode(windowHandle, GLFW_CURSOR, cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	Input::UpdateCursorEnabled(cursorEnabled);
}

void sf::Window::SetVsyncEnabled(bool enabled)
{
	vsyncEnabled = enabled;
	glfwSwapInterval(vsyncEnabled);
}

void sf::Window::SetToolBarEnabled(bool enabled)
{
	toolBarEnabled = enabled;
}

void sf::Window::AddOnResizeCallback(void(*newCallback)(void)) const
{
	onResizeCallbacks.push_back(newCallback);
}

void sf::Window::HandleImGuiViewports(void(*updatePlatformWindows)(), void(*renderPlatformWindows)(void*, void*))
{
	contextBackup = glfwGetCurrentContext();
	updatePlatformWindows();
	renderPlatformWindows(nullptr, nullptr);
	glfwMakeContextCurrent(contextBackup);
}

#ifdef SF_USE_OPENGL
bool sf::Window::ImGuiInitForOpenGL(bool(*initForOpenGL)(GLFWwindow*, bool))
{
	return initForOpenGL(windowHandle, true);
}
void* sf::Window::GetOpenGlFunctionAddress() const
{
	return (void*) glfwGetProcAddress;
}
#endif

#ifdef SF_USE_VULKAN
const char** sf::Window::GetVulkanExtensions(uint32_t& extensionCount) const
{
	return glfwGetRequiredInstanceExtensions(&extensionCount);
}
VkResult sf::Window::CreateVulkanSurface(VkInstance instance, VkSurfaceKHR& surfaceOut) const
{
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = glfwGetWin32Window(windowHandle);
	createInfo.hinstance = GetModuleHandle(nullptr);

	VkResult surfaceCreationResult = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surfaceOut);
	if (surfaceCreationResult != VK_SUCCESS)
	{
		std::cout << "[Window] Failed to create win32 surface\n";
		return surfaceCreationResult;
	}

	surfaceCreationResult = glfwCreateWindowSurface(instance, windowHandle, nullptr, &surfaceOut);
	if (surfaceCreationResult != VK_SUCCESS)
	{
		std::cout << "[Window] Failed to create surface on glfw window\n";
		return surfaceCreationResult;
	}
	return surfaceCreationResult;
}
#endif

void sf::Window::CursorPositionCallback(double xpos, double ypos)
{
	sf::Input::UpdateMousePosition(xpos, ypos);
}

void sf::Window::MouseButtonCallback(int button, int action, int mods)
{
	sf::Input::UpdateMouseButtons(button, action);
}

void sf::Window::KeyCallback(int key, int scancode, int action, int mods)
{
	sf::Input::UpdateKeyboard(key, action);
}

void sf::Window::CharacterCallback(uint32_t codepoint)
{
	sf::Input::UpdateCharacter(codepoint);
}

void sf::Window::ScrollCallback(double xoffset, double yoffset)
{
	sf::Input::UpdateMouseScroll(xoffset, yoffset);
}

void sf::Window::WindowResizeCallback(int width, int height)
{
	// height can be zero
	size.x = width;
	size.y = height;

	for (auto* function : onResizeCallbacks)
		function();
}

void sf::Window::Terminate()
{
	glfwTerminate();
}

void sf::Window::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) { windowMappingForCallbacks[window]->CursorPositionCallback(xpos, ypos); }
void sf::Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) { windowMappingForCallbacks[window]->MouseButtonCallback(button, action, mods); }
void sf::Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) { windowMappingForCallbacks[window]->KeyCallback(key, scancode, action, mods); }
void sf::Window::CharacterCallback(GLFWwindow* window, uint32_t codepoint) { windowMappingForCallbacks[window]->CharacterCallback(codepoint); }
void sf::Window::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) { windowMappingForCallbacks[window]->ScrollCallback(xoffset, yoffset); }
void sf::Window::WindowResizeCallback(GLFWwindow* window, int width, int height) { windowMappingForCallbacks[window]->WindowResizeCallback(width, height); }