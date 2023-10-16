#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include "VulkanDisplay.h"

namespace sf::Renderer::VulkanUtils
{
	void InsertImageMemoryBarrier(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		VkImageSubresourceRange subresourceRange);

	bool CreateShaderModuleFromBytes(const VulkanDisplay& vkDisplayData, const std::vector<char>& shaderCode, VkShaderModule& outShaderModule);
	bool CreateShaderModule(const VulkanDisplay& vkDisplayData, const std::string& shaderFilePath, VkShaderModule& outShaderModule);

	bool CreateBuffer(
		const VulkanDisplay& vkDisplayData,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);
	bool CreateVertexBuffer(
		const VulkanDisplay& vkDisplayData,
		VkDeviceSize size,
		const void* source,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);
	void CopyBuffer(const VulkanDisplay& vkDisplayData, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	uint32_t FindMemoryType(const VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
}