#pragma once

#define MAX_FRAMES_IN_FLIGHT 2

#include <VkBootstrap.h>
#include <Window.h>

namespace sf::Renderer
{
	struct VulkanDisplay
	{
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

		VkFormat depthFormat;
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;

		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		uint32_t currentFrameInFlight = 0;
		uint32_t swapchainImageIndex = 0;

		bool Initialize(const Window& windowArg, bool (*createPipelineFunc)(void));
		void Terminate(void (*destroyBuffersFunc)(void) = nullptr);

		bool CreateCommandPool();
		bool CreateSyncObjects();

		bool Predraw(const glm::vec4& clearColor);
		bool Postdraw();

		void CreateDepthResources(bool recreate = false);
		bool CreateSwapchain(bool recreate = false);
		bool RecreateSwapchain();

		inline VkImage SwapchainImage() { return swapchainImages[swapchainImageIndex]; }
		inline VkImageView SwapchainImageView() { return swapchainImageViews[swapchainImageIndex]; }

		inline uint32_t MaxFramesInFlight() { return MAX_FRAMES_IN_FLIGHT; }
		inline VkCommandBuffer CommandBuffer() { return commandBuffers[currentFrameInFlight]; }
		inline VkSemaphore ImageAvailableSemaphore() { return imageAvailableSemaphores[currentFrameInFlight]; }
		inline VkSemaphore RenderFinishedSemaphore() { return renderFinishedSemaphores[currentFrameInFlight]; }
		inline VkFence InFlightFence() { return inFlightFences[currentFrameInFlight]; }
	};
}