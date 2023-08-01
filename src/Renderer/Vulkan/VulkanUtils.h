#pragma once

#include <vulkan/vulkan.h>

namespace sf::Renderer::VulkanUtils
{
	bool IsSampleCountAvailable(VkPhysicalDevice pd, VkSampleCountFlagBits wantedSampleCount);
	bool IsSampleCountAvailable(VkPhysicalDeviceProperties pdProperties, VkSampleCountFlagBits wantedSampleCount);
	void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
		VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
		VkImage& image, VkDeviceMemory& imageMemory, VkDevice device, VkPhysicalDevice pd);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkDevice device);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice pd);
}