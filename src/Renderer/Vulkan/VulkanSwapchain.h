#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include <Window.h>

namespace sf::Renderer
{
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct VulkanSwapchain
	{
		const Window* window;

		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		// msaa
		VkSampleCountFlagBits msaaSamples;
		VkImage colorImage;
		VkDeviceMemory colorImageMemory;
		VkImageView colorImageView;

		VulkanSwapchain()=default;
		~VulkanSwapchain()=default;

		bool Create(const Window* window, VkPhysicalDevice physicalDeviceToUse, VkDevice device, VkSurfaceKHR surface, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex);
		bool CreateImageViews(VkDevice device);
		bool CreateFrameBuffers(VkDevice device, VkRenderPass renderPass);
		void CreateMultisampleResources(VkDevice device, VkPhysicalDevice pd);
		bool CreateRenderPass(VkDevice device, VkRenderPass& renderPass);
		VkViewport CreateViewport();
		inline VkSwapchainKHR Get() { return swapChain; };
		inline VkExtent2D GetExtent() { return swapChainExtent; }
		inline VkSampleCountFlagBits GetMsaaSamples() { return msaaSamples; }
		inline VkFramebuffer GetFramebuffer(uint32_t index) { return swapChainFramebuffers[index]; }
		void Recreate(const Window* window, VkPhysicalDevice physicalDeviceToUse, VkDevice device, VkSurfaceKHR surface, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex, VkRenderPass renderPass);
		void Destroy(VkDevice device);

	private:
		void Cleanup(VkDevice device);

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR targetSurface);
	};
}