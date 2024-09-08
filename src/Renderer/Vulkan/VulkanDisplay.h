#pragma once

#define MAX_FRAMES_IN_FLIGHT 2
#define ARRAY_LEN(a) sizeof(a) / sizeof(a[0])

#include <VkBootstrap.h>
#include <Window.h>

namespace sf::Renderer
{
	struct VulkanDisplay
	{
		struct FrameData
		{
			VkSemaphore imageAvailableSemaphore;
			VkSemaphore renderFinishedSemaphore;
			VkFence fence;
			VkCommandPool commandPool;
			VkCommandBuffer commandBuffer;
		};

		static VulkanDisplay* Instance;

		const Window* window;

		vkb::Instance instance;
		VkSurfaceKHR surface;
		vkb::Device device;
		vkb::DispatchTable disp;
		vkb::Swapchain swapchain;

		VkQueue graphicsQueue;
		VkQueue presentQueue;

		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;
		std::vector<VkFence> swapchainFences;

		VkFormat depthFormat;
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;

		FrameData frameData[MAX_FRAMES_IN_FLIGHT];

		uint32_t currentFrameInFlight = 0;
		uint32_t swapchainImageIndex = 0;
		uint32_t currentFrame = 0;

		bool Initialize(const Window& windowArg);
		void Terminate(void (*destroyBuffersFunc)(void) = nullptr, void (*destroyPipelinesFunc)(void) = nullptr);

		bool CreateCommandPool();
		bool CreateSyncObjects();

		bool Predraw(const glm::vec4& clearColor);
		bool Postdraw();

		void CreateDepthResources(bool recreate = false);
		bool CreateSwapchain(bool recreate = false);
		bool RecreateSwapchain();

		inline VkImage SwapchainImage() { return swapchainImages[swapchainImageIndex]; }
		inline VkImageView SwapchainImageView() { return swapchainImageViews[swapchainImageIndex]; }

		static inline VkDevice Device() { return Instance->device.device; }
		static inline VkPhysicalDevice PhysicalDevice() { return Instance->device.physical_device; }
		static inline VkQueue GraphicsQueue() { return Instance->graphicsQueue; }
		static inline uint32_t CurrentFrameInFlight() { return Instance->currentFrameInFlight; }

		static inline VkCommandPool CommandPool() { return Instance->frameData[CurrentFrameInFlight()].commandPool; }
		static inline VkCommandBuffer CommandBuffer() { return Instance->frameData[CurrentFrameInFlight()].commandBuffer; }
		static inline VkSemaphore ImageAvailableSemaphore() { return Instance->frameData[CurrentFrameInFlight()].imageAvailableSemaphore; }
		static inline VkSemaphore RenderFinishedSemaphore() { return Instance->frameData[CurrentFrameInFlight()].renderFinishedSemaphore; }
		static inline VkFence InFlightFence() { return Instance->frameData[CurrentFrameInFlight()].fence; }
	};
}