#include "VulkanDisplay.h"

#include <iostream>

#define MAX_FRAMES_IN_FLIGHT 2

bool sf::Renderer::VulkanDisplay::Initialize(const Window& windowArg, bool (*createPipelineFunc)(VulkanDisplay&))
{
	window = &windowArg;

	// Instance
	vkb::InstanceBuilder instance_builder;
	auto instance_builder_return = instance_builder.request_validation_layers().use_default_debug_messenger().build();
	if (!instance_builder_return)
	{
		std::cout << "[Renderer] Vulkan instance creation error: " << instance_builder_return.error().message() << std::endl;
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
		std::cout << "[Renderer] Failed to get vulkan physical device\n";
		return false;
	}
	vkb::PhysicalDevice vkb_physicalDevice = physical_device_selector_return.value();
	vkb::DeviceBuilder device_builder { vkb_physicalDevice };
	auto dev_ret = device_builder.build();
	if (!dev_ret)
	{
		std::cout << "[Renderer] Failed to create vulkan device\n";
		return false;
	}
	this->device = dev_ret.value();
	this->disp = this->device.make_table();

	std::cout << "[VulkanDisplay] Vulkan API version: " <<
		VK_API_VERSION_MAJOR(vkb_physicalDevice.properties.apiVersion) << '.' <<
		VK_API_VERSION_MINOR(vkb_physicalDevice.properties.apiVersion)  << std::endl;
	std::cout << "[VulkanDisplay] Vulkan driver version: " << vkb_physicalDevice.properties.driverVersion << std::endl;

	// Swapchain
	CreateSwapchain();

	// Get queues
	{
		auto queue_ret = this->device.get_queue(vkb::QueueType::graphics);
		if (!queue_ret)
		{
			std::cout << "[Renderer] Failed to get graphics queue\n";
			return false;
		}
		this->graphics_queue = queue_ret.value();
	}
	{
		auto queue_ret = this->device.get_queue(vkb::QueueType::present);
		if (!queue_ret)
		{
			std::cout << "[Renderer] Failed to get present queue\n";
			return false;
		}
		this->present_queue = queue_ret.value();
	}

	// Render pass
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = this->swapchain.image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	if (this->disp.createRenderPass(&render_pass_info, nullptr, &this->render_pass) != VK_SUCCESS)
	{
		std::cout << "failed to create render pass\n";
		return false; // failed to create render pass!
	}

	// Pipeline
	createPipelineFunc(*this);

	// Frame buffers
	CreateFramebuffers();

	// Command pool
	CreateCommandPool();

	// Command buffers
	CreateCommandBuffers();

	// Sync objects
	CreateSyncObjects();

	return true;
}

void sf::Renderer::VulkanDisplay::Terminate()
{
	vkDeviceWaitIdle(this->device.device);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		this->disp.destroySemaphore(this->finished_semaphore[i], nullptr);
		this->disp.destroySemaphore(this->available_semaphores[i], nullptr);
		this->disp.destroyFence(this->in_flight_fences[i], nullptr);
	}

	this->disp.destroyCommandPool(this->command_pool, nullptr);

	for (auto framebuffer : this->framebuffers)
		this->disp.destroyFramebuffer(framebuffer, nullptr);

	this->disp.destroyPipeline(this->graphics_pipeline, nullptr);
	this->disp.destroyPipelineLayout(this->pipeline_layout, nullptr);
	this->disp.destroyRenderPass(this->render_pass, nullptr);

	this->swapchain.destroy_image_views(this->swapchain_image_views);

	vkb::destroy_swapchain(this->swapchain);
	vkb::destroy_device(this->device);
	vkb::destroy_surface(this->instance, this->surface);
	vkb::destroy_instance(this->instance);
}

bool sf::Renderer::VulkanDisplay::CreateFramebuffers()
{
	this->swapchain_images = this->swapchain.get_images().value();
	this->swapchain_image_views = this->swapchain.get_image_views().value();

	this->framebuffers.resize(this->swapchain_image_views.size());

	for (size_t i = 0; i < this->swapchain_image_views.size(); i++) {
		VkImageView attachments[] = { this->swapchain_image_views[i] };

		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = this->render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = this->swapchain.extent.width;
		framebuffer_info.height = this->swapchain.extent.height;
		framebuffer_info.layers = 1;

		if (this->disp.createFramebuffer(&framebuffer_info, nullptr, &this->framebuffers[i]) != VK_SUCCESS) {
			return false; // failed to create framebuffer
		}
	}
	return true;
}

bool sf::Renderer::VulkanDisplay::CreateCommandPool()
{
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = this->device.get_queue_index(vkb::QueueType::graphics).value();

	if (this->disp.createCommandPool(&pool_info, nullptr, &this->command_pool) != VK_SUCCESS)
	{
		std::cout << "failed to create command pool\n";
		return false;
	}
	return true;
}

bool sf::Renderer::VulkanDisplay::CreateCommandBuffers()
{
	this->command_buffers.resize(this->framebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = this->command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)this->command_buffers.size();

	if (this->disp.allocateCommandBuffers(&allocInfo, this->command_buffers.data()) != VK_SUCCESS)
		return false;

	for (size_t i = 0; i < this->command_buffers.size(); i++)
	{
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (this->disp.beginCommandBuffer(this->command_buffers[i], &begin_info) != VK_SUCCESS)
			return false;

		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = this->render_pass;
		render_pass_info.framebuffer = this->framebuffers[i];
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = this->swapchain.extent;
		VkClearValue clearColor{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };
		render_pass_info.clearValueCount = 1;
		render_pass_info.pClearValues = &clearColor;

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

		this->disp.cmdSetViewport(this->command_buffers[i], 0, 1, &viewport);
		this->disp.cmdSetScissor(this->command_buffers[i], 0, 1, &scissor);
		this->disp.cmdBeginRenderPass(this->command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		this->disp.cmdBindPipeline(this->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphics_pipeline);
		this->disp.cmdDraw(this->command_buffers[i], 3, 1, 0, 0);
		this->disp.cmdEndRenderPass(this->command_buffers[i]);

		if (this->disp.endCommandBuffer(this->command_buffers[i]) != VK_SUCCESS)
		{
			std::cout << "failed to record command buffer\n";
			return false;
		}
	}
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
			std::cout << "failed to create sync objects\n";
			return false;
		}
	}
	return true;
}

bool sf::Renderer::VulkanDisplay::Display()
{
	this->disp.waitForFences(1, &this->in_flight_fences[this->current_frame], VK_TRUE, UINT64_MAX);

	uint32_t image_index = 0;
	VkResult result = this->disp.acquireNextImageKHR(
		this->swapchain, UINT64_MAX, this->available_semaphores[this->current_frame], VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		return RecreateSwapchain();

	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		std::cout << "failed to acquire swapchain image. Error " << result << "\n";
		return false;
	}

	if (this->image_in_flight[image_index] != VK_NULL_HANDLE)
		this->disp.waitForFences(1, &this->image_in_flight[image_index], VK_TRUE, UINT64_MAX);
	this->image_in_flight[image_index] = this->in_flight_fences[this->current_frame];

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { this->available_semaphores[this->current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
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
		std::cout << "failed to submit draw command buffer\n";
		return -1; //"failed to submit draw command buffer
	}

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swapChains[] = { this->swapchain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapChains;

	present_info.pImageIndices = &image_index;

	result = this->disp.queuePresentKHR(this->present_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		return RecreateSwapchain();
	else if (result != VK_SUCCESS)
	{
		std::cout << "failed to present swapchain image\n";
		return -1;
	}

	this->current_frame = (this->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	return 0;

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

	for (auto framebuffer : this->framebuffers)
		this->disp.destroyFramebuffer(framebuffer, nullptr);

	this->swapchain.destroy_image_views(this->swapchain_image_views);

	if (!CreateSwapchain()) return false;
	if (!CreateFramebuffers()) return false;
	if (!CreateCommandPool()) return false;
	if (!CreateCommandBuffers()) return false;
	return true;
}

