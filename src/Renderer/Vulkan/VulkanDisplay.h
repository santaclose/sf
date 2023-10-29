#pragma once

#define MAX_FRAMES_IN_FLIGHT 3

#include <VkBootstrap.h>
#include <Window.h>

namespace sf::Renderer
{
	struct VulkanDisplay
	{
		const Window* window;

		vkb::Instance instance;
		vkb::InstanceDispatchTable inst_disp;
		VkSurfaceKHR surface;
		vkb::Device device;
		vkb::DispatchTable disp;
		vkb::Swapchain swapchain;

		VkQueue graphics_queue;
		VkQueue present_queue;

		std::vector<VkImage> swapchain_images;
		std::vector<VkImageView> swapchain_image_views;

		VkFormat depthFormat;
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;

		VkPipelineLayout pipeline_layout;
		VkPipeline graphics_pipeline;

		VkCommandPool command_pool;
		std::vector<VkCommandBuffer> command_buffers;

		std::vector<VkSemaphore> available_semaphores;
		std::vector<VkSemaphore> finished_semaphore;
		std::vector<VkFence> in_flight_fences;
		std::vector<VkFence> image_in_flight;
		uint32_t current_frame = 0;
		uint32_t image_index = 0;

		bool Initialize(const Window& windowArg, bool (*createPipelineFunc)(void));
		void Terminate(void (*destroyBuffersFunc)(void) = nullptr);

		bool CreateCommandPool();
		bool CreateSyncObjects();

		bool Predraw(const glm::vec4& clearColor);
		bool Postdraw();

		void CreateDepthResources(bool recreate = false);
		bool CreateSwapchain(bool recreate = false);
		bool RecreateSwapchain();

		inline VkCommandBuffer GetCurrentCommandBuffer() { return command_buffers[image_index]; }
		inline uint32_t GetMaxFramesInFlight() { return MAX_FRAMES_IN_FLIGHT; }
	};
}