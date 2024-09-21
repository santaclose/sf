#ifdef SF_USE_VULKAN

#include "VulkanDisplay.h"

#include "VulkanUtils.h"
#include <iostream>

sf::Renderer::VulkanDisplay* sf::Renderer::VulkanDisplay::Instance = nullptr;

bool sf::Renderer::VulkanDisplay::Initialize(const Window& windowArg, unsigned int sampleCountArg)
{
	assert(Instance == nullptr);
	Instance = this;

	window = &windowArg;
	sampleCount = sampleCountArg;

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

	swapchainFences.resize(swapchain.image_count, VK_NULL_HANDLE);

	// Get queues
	{
		auto queue_ret = this->device.get_queue(vkb::QueueType::graphics);
		if (!queue_ret)
		{
			std::cout << "[VulkanDisplay] Failed to get graphics queue\n";
			return false;
		}
		this->graphicsQueue = queue_ret.value();
	}
	{
		auto queue_ret = this->device.get_queue(vkb::QueueType::present);
		if (!queue_ret)
		{
			std::cout << "[VulkanDisplay] Failed to get present queue\n";
			return false;
		}
		this->presentQueue = queue_ret.value();
	}

	// Frame buffers
	this->swapchainImages = this->swapchain.get_images().value();
	this->swapchainImageViews = this->swapchain.get_image_views().value();

	// Command pool
	CreateCommandPool();

	// Sync objects
	CreateSyncObjects();

	return true;
}

void sf::Renderer::VulkanDisplay::Terminate(void (*destroyBuffersFunc)(void), void (*destroyPipelinesFunc)(void))
{
	vkDeviceWaitIdle(this->device.device);
	for (int i = 0; i < ARRAY_LEN(frameData); i++)
	{
		this->disp.destroySemaphore(this->frameData[i].renderFinishedSemaphore, nullptr);
		this->disp.destroySemaphore(this->frameData[i].imageAvailableSemaphore, nullptr);
		this->disp.destroyFence(this->frameData[i].fence, nullptr);
		this->disp.destroyCommandPool(this->frameData[i].commandPool, nullptr);
	}

	if (destroyPipelinesFunc != nullptr)
		destroyPipelinesFunc();

	this->swapchain.destroy_image_views(this->swapchainImageViews);

	if (destroyBuffersFunc != nullptr)
		destroyBuffersFunc();

	vkDestroyImageView(this->device, depthImageView, nullptr);
	vkDestroyImage(this->device, depthImage, nullptr);
	vkFreeMemory(this->device, depthImageMemory, nullptr);

	vkDestroyImageView(this->device, multisampleImageView, nullptr);
	vkDestroyImage(this->device, multisampleImage, nullptr);
	vkFreeMemory(this->device, multisampleImageMemory, nullptr);

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

	for (int i = 0; i < ARRAY_LEN(frameData); i++)
	{
		if (this->disp.createCommandPool(&pool_info, nullptr, &this->frameData[i].commandPool) != VK_SUCCESS)
		{
			std::cout << "[VulkanDisplay] Failed to create command pool\n";
			return false;
		}
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = this->frameData[i].commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (this->disp.allocateCommandBuffers(&allocInfo, &(this->frameData[i].commandBuffer)) != VK_SUCCESS)
			return false;
	}
	return true;
}

bool sf::Renderer::VulkanDisplay::CreateSyncObjects()
{
	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < ARRAY_LEN(frameData); i++)
	{
		if (this->disp.createSemaphore(&semaphore_info, nullptr, &this->frameData[i].imageAvailableSemaphore) != VK_SUCCESS ||
			this->disp.createSemaphore(&semaphore_info, nullptr, &this->frameData[i].renderFinishedSemaphore) != VK_SUCCESS ||
			this->disp.createFence(&fence_info, nullptr, &this->frameData[i].fence) != VK_SUCCESS)
		{
			std::cout << "[VulkanDisplay] Failed to create sync objects\n";
			return false;
		}
	}
	return true;
}

bool sf::Renderer::VulkanDisplay::Predraw(const glm::vec4& clearColor)
{
	VkFence currentInFlightFence = InFlightFence();
	this->disp.waitForFences(1, &currentInFlightFence, VK_TRUE, UINT64_MAX);
	VkResult result = this->disp.acquireNextImageKHR(
		this->swapchain, UINT64_MAX, ImageAvailableSemaphore(), VK_NULL_HANDLE, &swapchainImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		return RecreateSwapchain();
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		std::cout << "[VulkanDisplay] Failed to acquire swapchain image. Error " << result << "\n";
		return false;
	}

	if (swapchainFences[swapchainImageIndex] != VK_NULL_HANDLE)
		this->disp.waitForFences(1, &swapchainFences[swapchainImageIndex], VK_TRUE, UINT64_MAX);
	swapchainFences[swapchainImageIndex] = currentInFlightFence;

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (this->disp.beginCommandBuffer(CommandBuffer(), &begin_info) != VK_SUCCESS)
		return false;

	VulkanUtils::InsertImageMemoryBarrier(
		CommandBuffer(),
		SwapchainImage(),
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	VkRenderingAttachmentInfo colorInfo{};
	colorInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorInfo.imageView = multisampleImageView;
	colorInfo.resolveImageView = SwapchainImageView();
	colorInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	colorInfo.resolveImageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	colorInfo.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
	colorInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorInfo.clearValue.color = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
	VkRenderingAttachmentInfo depthInfo{};
	depthInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthInfo.imageView = depthImageView;
	depthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	depthInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthInfo.clearValue.depthStencil.depth = 1.0f;
	VkRenderingInfo renderInfo{};
	renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderInfo.renderArea = { 0, 0, this->swapchain.extent.width, this->swapchain.extent.height };
	renderInfo.layerCount = 1;
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pDepthAttachment = &depthInfo;
	renderInfo.pColorAttachments = &colorInfo;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = (float)this->swapchain.extent.height;
	viewport.width = (float)this->swapchain.extent.width;
	viewport.height = -(float)this->swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = this->swapchain.extent;

	this->disp.cmdBeginRendering(CommandBuffer(), &renderInfo);
	this->disp.cmdSetViewport(CommandBuffer(), 0, 1, &viewport);
	this->disp.cmdSetScissor(CommandBuffer(), 0, 1, &scissor);

	this->disp.resetFences(1, &currentInFlightFence);

	return true;
}

bool sf::Renderer::VulkanDisplay::Postdraw()
{
	this->disp.cmdEndRendering(CommandBuffer());

	VulkanUtils::InsertImageMemoryBarrier(
		CommandBuffer(),
		SwapchainImage(),
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	if (this->disp.endCommandBuffer(CommandBuffer()) != VK_SUCCESS)
	{
		std::cout << "[VulkanDisplay] Failed to record command buffer\n";
		return false;
	}

	VkSemaphore wait_semaphores[] = { ImageAvailableSemaphore() };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = {};
	VkCommandBuffer cmdBuffer = CommandBuffer();
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = wait_semaphores;
	submitInfo.pWaitDstStageMask = wait_stages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	VkSemaphore signal_semaphores[] = { RenderFinishedSemaphore() };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signal_semaphores;

	if (this->disp.queueSubmit(this->graphicsQueue, 1, &submitInfo, InFlightFence()) != VK_SUCCESS)
	{
		std::cout << "[VulkanDisplay] Failed to submit draw command buffer\n";
		return false;
	}

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &this->swapchain.swapchain;
	present_info.pImageIndices = &swapchainImageIndex;

	VkResult result = this->disp.queuePresentKHR(this->presentQueue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		return RecreateSwapchain();
	else if (result != VK_SUCCESS)
	{
		std::cout << "[VulkanDisplay] Failed to present swapchain image\n";
		return false;
	}

	currentFrameInFlight = (currentFrameInFlight + 1) % MAX_FRAMES_IN_FLIGHT;
	currentFrame++;
	return true;
}

void sf::Renderer::VulkanDisplay::CreateMultisampleResources(bool recreate)
{
	if (recreate)
	{
		vkDestroyImageView(this->device, multisampleImageView, nullptr);
		vkDestroyImage(this->device, multisampleImage, nullptr);
		vkFreeMemory(this->device, multisampleImageMemory, nullptr);
	}
	VulkanUtils::CreateImage(this->swapchain.extent.width, this->swapchain.extent.height, this->swapchain.image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, multisampleImage, multisampleImageMemory, (VkSampleCountFlagBits)sampleCount);
	multisampleImageView = VulkanUtils::CreateImageView(multisampleImage, this->swapchain.image_format, VK_IMAGE_ASPECT_COLOR_BIT);
}

void sf::Renderer::VulkanDisplay::CreateDepthResources(bool recreate)
{
	depthFormat = VulkanUtils::FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
	if (recreate)
	{
		vkDestroyImageView(this->device, depthImageView, nullptr);
		vkDestroyImage(this->device, depthImage, nullptr);
		vkFreeMemory(this->device, depthImageMemory, nullptr);
	}

	VulkanUtils::CreateImage(this->swapchain.extent.width, this->swapchain.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, (VkSampleCountFlagBits)sampleCount);
	depthImageView = VulkanUtils::CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

bool sf::Renderer::VulkanDisplay::CreateSwapchain(bool recreate)
{
	vkb::SwapchainBuilder swapchain_builder{ this->device };
	auto swap_ret = swapchain_builder.set_old_swapchain(this->swapchain)
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.build();
	if (!swap_ret)
	{
		std::cout << swap_ret.error().message() << " " << swap_ret.vk_result() << std::endl;
		return false;
	}
	vkb::destroy_swapchain(this->swapchain);
	this->swapchain = swap_ret.value();

	CreateMultisampleResources(recreate);
	CreateDepthResources(recreate);
	std::cout << "[VulkanDisplay] Created swapchain with " << this->swapchain.image_count << " images\n";
	return true;
}

bool sf::Renderer::VulkanDisplay::RecreateSwapchain()
{
	this->disp.deviceWaitIdle();
	for (int i = 0; i < ARRAY_LEN(frameData); i++)
		this->disp.destroyCommandPool(this->frameData[i].commandPool, nullptr);
	this->swapchain.destroy_image_views(this->swapchainImageViews);

	if (!CreateSwapchain(true))
		return false;

	this->swapchainImages = this->swapchain.get_images().value();
	this->swapchainImageViews = this->swapchain.get_image_views().value();

	if (!CreateCommandPool())
		return false;

	return true;
}

#endif