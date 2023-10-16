#include "VulkanUtils.h"

#include <iostream>

#include <FileUtils.h>

void VulkanUtils::InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)
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

bool VulkanUtils::CreateShaderModuleFromBytes(const VkDevice& device, const std::vector<char>& shaderBytes, VkShaderModule& outShaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderBytes.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBytes.data());
	if (vkCreateShaderModule(device, &createInfo, nullptr, &outShaderModule) != VK_SUCCESS)
	{
		std::cout << "[VulkanUtils] Could not create shader module from source:\n" << shaderBytes.data() << std::endl;
		return false;
	}
	return true;
}

bool VulkanUtils::CreateShaderModule(const VkDevice& device, const std::string& shaderFilePath, VkShaderModule& outShaderModule)
{
	std::vector<char> shaderBytes;
	if (!sf::FileUtils::ReadFileAsBytes(shaderFilePath, shaderBytes))
	{
		std::cout << "[VulkanUtils] Could not read shader file: " << shaderFilePath << std::endl;
		return false;
	}
	return CreateShaderModuleFromBytes(device, shaderBytes, outShaderModule);
}

uint32_t VulkanUtils::FindMemoryType(const VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	std::cout << "[VulkanUtils] Failed to find suitable memory type\n";
}