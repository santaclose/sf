#include "VulkanDisplay.h"

#include "VulkanUtils.h"
#include <iostream>

#define MAX_FRAMES_IN_FLIGHT 3

bool sf::Renderer::VulkanDisplay::Initialize(const Window& windowArg, bool (*createPipelineFunc)(VulkanDisplay&))
{
	window = &windowArg;

	// Instance
	vkb::InstanceBuilder instance_builder;
	auto instance_builder_return = instance_builder.request_validation_layers().desire_api_version(1, 3).use_default_debug_messenger().build();
	if (!instance_builder_return)
	{
		std::cout << "[VulkanDisplay] Vulkan instance creation error: " << instance_builder_return.error().message() << std::endl;
		return false;
	}
	this->instance = instance_builder_return.value();

	// Create surface
	window->CreateVulkanSurface(this->instance.instance, this->surface);

	// Device
	vkb::PhysicalDeviceSelector phys_device_selector(this->instance);
	auto physical_device_selector_return = phys_device_selector
		.set_surface(this->surface)
		.select();
	if (!physical_device_selector_return)
	{
		std::cout << "[VulkanDisplay] Failed to get vulkan physical device\n";
		return false;
	}
	vkb::PhysicalDevice vkb_physicalDevice = physical_device_selector_return.value();
	vkb::DeviceBuilder device_builder { vkb_physicalDevice };

	VkPhysicalDeviceVulkan13Features vk13features{};
	vk13features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	vk13features.dynamicRendering = true;
	auto dev_ret = device_builder.add_pNext(&vk13features).build();
	if (!dev_ret)
	{
		std::cout << "[VulkanDisplay] Failed to create vulkan device\n";
		return false;
	}
	this->device = dev_ret.value();
	this->disp = this->device.make_table();

	std::cout << "[VulkanDisplay] Using GPU: " << vkb_physicalDevice.name << std::endl;
	std::cout << "[VulkanDisplay] Vulkan API version: " <<
		VK_API_VERSION_MAJOR(vkb_physicalDevice.properties.apiVersion) << '.' <<
		VK_API_VERSION_MINOR(vkb_physicalDevice.properties.apiVersion) << std::endl;
	std::cout << "[VulkanDisplay] Vulkan driver version: " << vkb_physicalDevice.properties.driverVersion << std::endl;

	// Swapchain
	CreateSwapchain();

	// Get queues
	{
		auto queue_ret = this->device.get_queue(vkb::QueueType::graphics);
		if (!queue_ret)
		{
			std::cout << "[VulkanDisplay] Failed to get graphics queue\n";
			return false;
		}
		this->graphics_queue = queue_ret.value();
	}
	{
		auto queue_ret = this->device.get_queue(vkb::QueueType::present);
		if (!queue_ret)
		{
			std::cout << "[VulkanDisplay] Failed to get present queue\n";
			return false;
		}
		this->present_queue = queue_ret.value();
	}

	// Pipeline
	createPipelineFunc(*this);

	// Frame buffers
	this->command_buffers.resize(this->swapchain.image_count);
	this->swapchain_images = this->swapchain.get_images().value();
	this->swapchain_image_views = this->swapchain.get_image_views().value();

	// Command pool
	CreateCommandPool();

	// Sync objects
	CreateSyncObjects();

	return true;
}

void sf::Renderer::VulkanDisplay::Terminate(void (*destroyBuffersFunc)(void))
{
	vkDeviceWaitIdle(this->device.device);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		this->disp.destroySemaphore(this->finished_semaphore[i], nullptr);
		this->disp.destroySemaphore(this->available_semaphores[i], nullptr);
		this->disp.destroyFence(this->in_flight_fences[i], nullptr);
	}

	this->disp.destroyCommandPool(this->command_pool, nullptr);
	this->disp.destroyPipeline(this->graphics_pipeline, nullptr);
	this->disp.destroyPipelineLayout(this->pipeline_layout, nullptr);

	this->swapchain.destroy_image_views(this->swapchain_image_views);

	if (destroyBuffersFunc != nullptr)
		destroyBuffersFunc();

	vkb::destroy_swapchain(this->swapchain);
	vkb::destroy_device(this->device);
	vkb::destroy_surface(this->instance, this->surface);
	vkb::destroy_instance(this->instance);
}

bool sf::Renderer::VulkanDisplay::CreateCommandPool()
{
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = this->device.get_queue_index(vkb::QueueType::graphics).value();

	if (this->disp.createCommandPool(&pool_info, nullptr, &this->command_pool) != VK_SUCCESS)
	{
		std::cout << "[VulkanDisplay] Failed to create command pool\n";
		return false;
	}

	// Command buffers
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = this->command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = this->swapchain.image_count;

	if (this->disp.allocateCommandBuffers(&allocInfo, this->command_buffers.data()) != VK_SUCCESS)
		return false;

	return true;
}

bool sf::Renderer::VulkanDisplay::CreateSyncObjects()
{
	this->available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	this->finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
	this->in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	this->image_in_flight.resize(this->swapchain.image_count, VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (this->disp.createSemaphore(&semaphore_info, nullptr, &this->available_semaphores[i]) != VK_SUCCESS ||
			this->disp.createSemaphore(&semaphore_info, nullptr, &this->finished_semaphore[i]) != VK_SUCCESS ||
			this->disp.createFence(&fence_info, nullptr, &this->in_flight_fences[i]) != VK_SUCCESS)
		{
			std::cout << "[VulkanDisplay] Failed to create sync objects\n";
			return false;
		}
	}
	return true;
}

bool sf::Renderer::VulkanDisplay::PreDraw()
{
	this->disp.waitForFences(1, &this->in_flight_fences[this->current_frame], VK_TRUE, UINT64_MAX);

	VkResult result = this->disp.acquireNextImageKHR(
		this->swapchain, UINT64_MAX, this->available_semaphores[this->current_frame], VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		return RecreateSwapchain();
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		std::cout << "[VulkanDisplay] Failed to acquire swapchain image. Error " << result << "\n";
		return false;
	}

	if (this->image_in_flight[image_index] != VK_NULL_HANDLE)
		this->disp.waitForFences(1, &this->image_in_flight[image_index], VK_TRUE, UINT64_MAX);
	this->image_in_flight[image_index] = this->in_flight_fences[this->current_frame];

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (this->disp.beginCommandBuffer(this->command_buffers[image_index], &begin_info) != VK_SUCCESS)
		return false;

	VulkanUtils::InsertImageMemoryBarrier(
		this->command_buffers[image_index],
		this->swapchain_images[image_index],
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	VkRenderingAttachmentInfo colorInfo{};
	colorInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorInfo.imageView = this->swapchain_image_views[current_frame];
	colorInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	colorInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorInfo.clearValue.color = { 0.0f,0.2f,0.0f,0.0f };

	VkRenderingInfo renderInfo{};
	renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderInfo.renderArea = { 0, 0, this->swapchain.extent.width, this->swapchain.extent.height };
	renderInfo.layerCount = 1;
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments = &colorInfo;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)this->swapchain.extent.width;
	viewport.height = (float)this->swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = this->swapchain.extent;

	this->disp.cmdBeginRendering(this->command_buffers[image_index], &renderInfo);
	this->disp.cmdSetViewport(this->command_buffers[image_index], 0, 1, &viewport);
	this->disp.cmdSetScissor(this->command_buffers[image_index], 0, 1, &scissor);
	this->disp.cmdBindPipeline(this->command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphics_pipeline);

	return true;
}

bool sf::Renderer::VulkanDisplay::PostDraw()
{
	this->disp.cmdEndRendering(this->command_buffers[image_index]);

	VulkanUtils::InsertImageMemoryBarrier(
		this->command_buffers[image_index],
		this->swapchain_images[image_index],
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	if (this->disp.endCommandBuffer(this->command_buffers[image_index]) != VK_SUCCESS)
	{
		std::cout << "[VulkanDisplay] Failed to record command buffer\n";
		return false;
	}

	VkSemaphore wait_semaphores[] = { this->available_semaphores[this->current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = wait_semaphores;
	submitInfo.pWaitDstStageMask = wait_stages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->command_buffers[image_index];

	VkSemaphore signal_semaphores[] = { this->finished_semaphore[this->current_frame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signal_semaphores;

	this->disp.resetFences(1, &this->in_flight_fences[this->current_frame]);

	if (this->disp.queueSubmit(this->graphics_queue, 1, &submitInfo, this->in_flight_fences[this->current_frame]) != VK_SUCCESS)
	{
		std::cout << "[VulkanDisplay] Failed to submit draw command buffer\n";
		return false;
	}

	VkSwapchainKHR swapChains[] = { this->swapchain };
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapChains;
	present_info.pImageIndices = &image_index;

	VkResult result = this->disp.queuePresentKHR(this->present_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		return RecreateSwapchain();
	else if (result != VK_SUCCESS)
	{
		std::cout << "[VulkanDisplay] Failed to present swapchain image\n";
		return false;
	}

	this->current_frame = (this->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

	return true;
}

bool sf::Renderer::VulkanDisplay::CreateSwapchain()
{
	vkb::SwapchainBuilder swapchain_builder{ this->device };
	auto swap_ret = swapchain_builder.set_old_swapchain(this->swapchain).build();
	if (!swap_ret)
	{
		std::cout << swap_ret.error().message() << " " << swap_ret.vk_result() << std::endl;
		return false;
	}
	vkb::destroy_swapchain(this->swapchain);
	this->swapchain = swap_ret.value();
	return true;
}

bool sf::Renderer::VulkanDisplay::RecreateSwapchain()
{
	this->disp.deviceWaitIdle();
	this->disp.destroyCommandPool(this->command_pool, nullptr);
	this->swapchain.destroy_image_views(this->swapchain_image_views);

	if (!CreateSwapchain()) return false;

	this->command_buffers.resize(this->swapchain.image_count);
	this->swapchain_images = this->swapchain.get_images().value();
	this->swapchain_image_views = this->swapchain.get_image_views().value();

	if (!CreateCommandPool()) return false;

	return true;
}