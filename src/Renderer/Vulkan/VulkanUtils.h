#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan.h>

namespace VulkanUtils
{
	void InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer,
		VkImage image,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		VkImageSubresourceRange subresourceRange);

	bool CreateShaderModuleFromBytes(const VkDevice& device, const std::vector<char>& shaderCode, VkShaderModule& outShaderModule);
	bool CreateShaderModule(const VkDevice& device, const std::string& shaderFilePath, VkShaderModule& outShaderModule);
}