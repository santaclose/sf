#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

#include <GameInitializationData.h>

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
		Window(const GameInitializationData& gameInitData);
		~Window();
		void PollEvents();
		void SwapBuffers();
		double GetTime();
		bool ShouldClose();

		void SetFullScreenEnabled(bool enabled);
		void SetCursorEnabled(bool enabled);
		void SetVsyncEnabled(bool enabled);
		void SetToolBarEnabled(bool enabled);
		inline bool GetFullScreenEnabled() const { return fullscreenEnabled; }
		inline bool GetCursorEnabled() const { return cursorEnabled; }
		inline bool GetVsyncEnabled() const { return vsyncEnabled; }
		inline bool GetToolBarEnabled() const { return toolBarEnabled; }

		void AddOnResizeCallback(void (*newCallback)(void)) const;

		inline glm::uvec2 GetSize() const { return size; };
		inline uint32_t GetWidth() const { return size.x; };
		inline uint32_t GetHeight() const { return size.y; };

		void HandleImGuiViewports(void (*updatePlatformWindows)(), void (*renderPlatformWindows)(void*, void*));

#ifdef SF_USE_OPENGL
		bool ImGuiInitForOpenGL(bool (*initForOpenGL)(GLFWwindow*, bool));
		void* GetOpenGlFunctionAddress() const;
#endif
#ifdef SF_USE_VULKAN
		const char** GetVulkanExtensions(uint32_t& extensionCount) const;
		VkResult CreateVulkanSurface(VkInstance instance, VkSurfaceKHR& surfaceOut) const;
#endif

	private:
		GLFWwindow* windowHandle;
		GLFWwindow* contextBackup; // used for imgui viewports

		std::string title;
		glm::uvec2 size = { 0, 0 };
		uint32_t msaaCount = 4;
		bool fullscreenEnabled = false;
		bool toolBarEnabled = true;
		bool cursorEnabled = true;
		bool vsyncEnabled = true;

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