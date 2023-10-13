#pragma once

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
		std::vector<VkFramebuffer> framebuffers;

		VkRenderPass render_pass;
		VkPipelineLayout pipeline_layout;
		VkPipeline graphics_pipeline;

		VkCommandPool command_pool;
		std::vector<VkCommandBuffer> command_buffers;

		std::vector<VkSemaphore> available_semaphores;
		std::vector<VkSemaphore> finished_semaphore;
		std::vector<VkFence> in_flight_fences;
		std::vector<VkFence> image_in_flight;
		uint32_t current_frame = 0;

		bool Initialize(const Window& windowArg, bool (*createPipelineFunc)(VulkanDisplay&));
		void Terminate();

		bool CreateFramebuffers();
		bool CreateCommandPool();
		bool CreateCommandBuffers();
		bool CreateSyncObjects();

		bool Display();

		bool CreateSwapchain();
		bool RecreateSwapchain();
	};
}