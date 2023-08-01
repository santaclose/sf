#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

#ifdef SF_PLATFORM_WINDOWS
struct HWND__;
typedef HWND__* HWND;
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;
typedef HINSTANCE HMODULE;
#endif

#ifdef SF_USE_VULKAN
enum VkResult;

struct VkInstance_T;
typedef struct VkInstance_T* VkInstance;
struct VkSurfaceKHR_T;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
#endif

struct GLFWwindow;

namespace sf
{
	struct Window
	{
		Window(const std::string& title = "sf window", const glm::uvec2& size = {1280, 720}, bool fullscreenEnabled = false, bool cursorEnabled = true, bool vsyncEnabled = true);
		~Window();
		void PollEvents();
		void SwapBuffers();
		double GetTime();
		bool ShouldClose();

		void SetFullScreenEnabled(bool enabled);
		void SetCursorEnabled(bool enabled);
		void SetVsyncEnabled(bool enabled);

		void AddOnResizeCallback(void (*newCallback)(void)) const;

		glm::uvec2 GetSize() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

#ifdef SF_USE_OPENGL
		void* GetOpenGlFunctionAddress() const;
#endif
#ifdef SF_USE_VULKAN
		const char** GetVulkanExtensions(uint32_t& extensionCount) const;
		VkResult CreateVulkanSurface(VkInstance instance, VkSurfaceKHR& surfaceOut) const;
#endif

	private:
		std::string title;
		glm::uvec2 size = { 0,0 };
		bool fullscreenEnabled = false;
		bool cursorEnabled = true;
		bool vsyncEnabled = true;

		GLFWwindow* windowHandle;

		mutable std::vector<void (*)()> onResizeCallbacks;
		void CursorPositionCallback(double xpos, double ypos);
		void MouseButtonCallback(int button, int action, int mods);
		void KeyCallback(int key, int scancode, int action, int mods);
		void CharacterCallback(uint32_t codepoint);
		void ScrollCallback(double xoffset, double yoffset);
		void WindowResizeCallback(int width, int height);

		// Callbacks
		static std::unordered_map<GLFWwindow*, Window*> windowMappingForCallbacks;
		static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void CharacterCallback(GLFWwindow* window, uint32_t codepoint);
		static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void WindowResizeCallback(GLFWwindow* window, int width, int height);
	};
}