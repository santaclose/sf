#ifdef SF_USE_VULKAN

#include "VulkanUtils.h"

#include <iostream>

#include <FileUtils.h>

void sf::Renderer::VulkanUtils::InsertImageMemoryBarrier(
	VkCommandBuffer cmdbuffer,
	VkImage image,
	VkAccessFlags srcAccessMask,
	VkAccessFlags dstAccessMask,
	VkImageLayout oldImageLayout,
	VkImageLayout newImageLayout,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)
{
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.srcAccessMask = srcAccessMask;
	imageMemoryBarrier.dstAccessMask = dstAccessMask;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(
		cmdbuffer,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);
}

VkFormat sf::Renderer::VulkanUtils::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(VulkanDisplay::PhysicalDevice(), format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			return format;
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			return format;
	}

	std::cout << "[VulkanUtils] Failed to find supported format\n";
	return (VkFormat)0;
}

VkImageView sf::Renderer::VulkanUtils::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(VulkanDisplay::Device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		std::cout << "[VulkanUtils] Failed to create texture image view\n";

	return imageView;
}

void sf::Renderer::VulkanUtils::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(VulkanDisplay::Device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		std::cout << "[VulkanUtils] Failed to create image\n";

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(VulkanDisplay::Device(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(VulkanDisplay::Device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		std::cout << "[VulkanUtils] Failed to allocate image memory\n";

	vkBindImageMemory(VulkanDisplay::Device(), image, imageMemory, 0);
}

bool sf::Renderer::VulkanUtils::CreateShaderModuleFromBytes(const std::vector<char>& shaderBytes, VkShaderModule& outShaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderBytes.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBytes.data());
	if (vkCreateShaderModule(VulkanDisplay::Device(), &createInfo, nullptr, &outShaderModule) != VK_SUCCESS)
	{
		std::cout << "[VulkanUtils] Could not create shader module from source:\n" << shaderBytes.data() << std::endl;
		return false;
	}
	return true;
}

bool sf::Renderer::VulkanUtils::CreateShaderModule(const std::string& shaderFilePath, VkShaderModule& outShaderModule)
{
	std::vector<char> shaderBytes;
	if (!sf::FileUtils::ReadFileAsBytes(shaderFilePath, shaderBytes))
	{
		std::cout << "[VulkanUtils] Could not read shader file: " << shaderFilePath << std::endl;
		return false;
	}
	return CreateShaderModuleFromBytes(shaderBytes, outShaderModule);
}

bool sf::Renderer::VulkanUtils::CreateBuffer(
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VulkanDisplay::Device(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		std::cout << "[VulkanUtils] Failed to create buffer" << std::endl;
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(VulkanDisplay::Device(), buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(VulkanDisplay::Device(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		std::cout << "[VulkanUtils] Failed to allocate buffer memory" << std::endl;
		return false;
	}

	vkBindBufferMemory(VulkanDisplay::Device(), buffer, bufferMemory, 0);
	return true;
}

bool sf::Renderer::VulkanUtils::CreateVertexBuffer(
	VkDeviceSize size,
	const void* source,
	VkBuffer& buffer, 
	VkDeviceMemory& bufferMemory)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	if (!CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory))
		return false;

	void* data;
	vkMapMemory(VulkanDisplay::Device(), stagingBufferMemory, 0, size, 0, &data);
	memcpy(data, source, (size_t)size);
	vkUnmapMemory(VulkanDisplay::Device(), stagingBufferMemory);

	if (!CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		buffer, bufferMemory))
		return false;

	CopyBuffer(stagingBuffer, buffer, size);

	vkDestroyBuffer(VulkanDisplay::Device(), stagingBuffer, nullptr);
	vkFreeMemory(VulkanDisplay::Device(), stagingBufferMemory, nullptr);
	return true;
}

bool sf::Renderer::VulkanUtils::CreateIndexBuffer(
	VkDeviceSize size,
	const void* source,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	if (!CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory))
		return false;

	void* data;
	vkMapMemory(VulkanDisplay::Device(), stagingBufferMemory, 0, size, 0, &data);
	memcpy(data, source, (size_t)size);
	vkUnmapMemory(VulkanDisplay::Device(), stagingBufferMemory);

	if (!CreateBuffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		buffer, bufferMemory))
		return false;

	CopyBuffer(stagingBuffer, buffer, size);

	vkDestroyBuffer(VulkanDisplay::Device(), stagingBuffer, nullptr);
	vkFreeMemory(VulkanDisplay::Device(), stagingBufferMemory, nullptr);
}

void sf::Renderer::VulkanUtils::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = VulkanDisplay::CommandPool();
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(VulkanDisplay::Device(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(VulkanDisplay::GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(VulkanDisplay::GraphicsQueue());

	vkFreeCommandBuffers(VulkanDisplay::Device(), VulkanDisplay::CommandPool(), 1, &commandBuffer);
}

uint32_t sf::Renderer::VulkanUtils::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(VulkanDisplay::PhysicalDevice(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	std::cout << "[VulkanUtils] Failed to find suitable memory type\n";
}

#endif