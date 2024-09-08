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

	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	inline bool HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	bool CreateShaderModuleFromBytes(const std::vector<char>& shaderCode, VkShaderModule& outShaderModule);
	bool CreateShaderModule(const std::string& shaderFilePath, VkShaderModule& outShaderModule);

	bool CreateBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);
	bool CreateVertexBuffer(
		VkDeviceSize size,
		const void* source,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);
	bool CreateIndexBuffer(
		VkDeviceSize size,
		const void* source,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
}