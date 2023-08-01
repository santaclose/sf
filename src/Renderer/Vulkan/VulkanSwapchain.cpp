#include "VulkanSwapchain.h"

#include "VulkanUtils.h"

#include <iostream>
#include <algorithm>


bool sf::Renderer::VulkanSwapchain::Create(const Window* window, VkPhysicalDevice physicalDeviceToUse, VkDevice device, VkSurfaceKHR surface, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex)
{
	this->window = window;
	msaaSamples = VulkanUtils::IsSampleCountAvailable(physicalDeviceToUse, VK_SAMPLE_COUNT_4_BIT) ? VK_SAMPLE_COUNT_4_BIT : VK_SAMPLE_COUNT_1_BIT;

	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDeviceToUse, surface);
	if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
	{
		std::cout << "[VulkanSwapchain] No swap chain support for device\n";
		return false;
	}

	// Choose format
	VkSurfaceFormatKHR formatToUse = swapChainSupport.formats[0]; // in case none meet condition below
	for (const auto& availableFormat : swapChainSupport.formats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			formatToUse = availableFormat;
			break;
		}
	}

	// Choose presentation mode
	VkPresentModeKHR presentModeToUse = VK_PRESENT_MODE_FIFO_KHR;

	// Choose swap extent
	VkExtent2D swapExtentToUse;
	uint32_t maxint = (std::numeric_limits<uint32_t>::max)();
	if (swapChainSupport.capabilities.currentExtent.width != maxint)
		swapExtentToUse = swapChainSupport.capabilities.currentExtent;
	else
	{
		swapExtentToUse = { static_cast<uint32_t>(window->GetWidth()), static_cast<uint32_t>(window->GetHeight()) };
		swapExtentToUse.width = std::clamp(swapExtentToUse.width, swapChainSupport.capabilities.minImageExtent.width, swapChainSupport.capabilities.maxImageExtent.width);
		swapExtentToUse.height = std::clamp(swapExtentToUse.height, swapChainSupport.capabilities.minImageExtent.height, swapChainSupport.capabilities.maxImageExtent.height);
	}

	// Choose image count
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = formatToUse.format;
	createInfo.imageColorSpace = formatToUse.colorSpace;
	createInfo.imageExtent = swapExtentToUse;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
	if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentModeToUse;
	createInfo.clipped = VK_TRUE; // don't care about pixels behind other windows
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult swapchainCreationResult = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
	if (swapchainCreationResult != VK_SUCCESS)
	{
		// can fail if glfwCreateWindowSurface is called multiple times
		std::cout << "[VulkanSwapchain] Failed to create swap chain\n";
		return false;
	}
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	swapChainImageFormat = formatToUse.format;
	swapChainExtent = swapExtentToUse;
}

void sf::Renderer::VulkanSwapchain::Recreate(const Window* window, VkPhysicalDevice physicalDeviceToUse, VkDevice device, VkSurfaceKHR surface, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex, VkRenderPass renderPass)
{
	Cleanup(device);
	Create(window, physicalDeviceToUse, device, surface, graphicsQueueFamilyIndex, presentQueueFamilyIndex);
	CreateImageViews(device);
	CreateMultisampleResources(device, physicalDeviceToUse);
	CreateFrameBuffers(device, renderPass);
}

void sf::Renderer::VulkanSwapchain::Destroy(VkDevice device)
{
	Cleanup(device);
	for (auto framebuffer : swapChainFramebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	for (auto imageView : swapChainImageViews)
		vkDestroyImageView(device, imageView, nullptr);
	vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void sf::Renderer::VulkanSwapchain::Cleanup(VkDevice device)
{
	vkDestroyImageView(device, colorImageView, nullptr);
	vkDestroyImage(device, colorImage, nullptr);
	vkFreeMemory(device, colorImageMemory, nullptr);
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
	for (size_t i = 0; i < swapChainImageViews.size(); i++)
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	vkDestroySwapchainKHR(device, swapChain, nullptr);
}

bool sf::Renderer::VulkanSwapchain::CreateImageViews(VkDevice device)
{
	swapChainImageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult imageViewCreationResult = vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]);
		if (imageViewCreationResult != VK_SUCCESS)
		{
			std::cout << "[VulkanSwapchain] Failed to create image view\n";
			return false;
		}
	}
}

bool sf::Renderer::VulkanSwapchain::CreateFrameBuffers(VkDevice device, VkRenderPass renderPass)
{
	swapChainFramebuffers.resize(swapChainImageViews.size());
	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::vector<VkImageView> attachments = {
			colorImageView,
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			std::cout << "[VulkanSwapchain] Failed to create framebuffer\n";
			return false;
		}
	}
}

void sf::Renderer::VulkanSwapchain::CreateMultisampleResources(VkDevice device, VkPhysicalDevice pd)
{
	VkFormat colorFormat = swapChainImageFormat;
	VulkanUtils::CreateImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory, device, pd);
	colorImageView = VulkanUtils::CreateImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, device);
}

bool sf::Renderer::VulkanSwapchain::CreateRenderPass(VkDevice device, VkRenderPass& renderPass)
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 1;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	std::vector<VkAttachmentDescription> attachments = { colorAttachment, colorAttachmentResolve };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		std::cout << "[SwapChain] Failed to create render pass\n";
		return false;
	}
	return true;
}

VkViewport sf::Renderer::VulkanSwapchain::CreateViewport()
{
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	return viewport;
}

sf::Renderer::SwapChainSupportDetails sf::Renderer::VulkanSwapchain::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR targetSurface)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, targetSurface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, targetSurface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, targetSurface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, targetSurface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, targetSurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}
